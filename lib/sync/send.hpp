#ifndef _MSYNC_SEND_HPP_
#define _MSYNC_SEND_HPP_

#include <print_logger.hpp>
#include <filesystem.hpp>

#include <string>
#include <string_view>
#include <algorithm>
#include <utility>
#include <tuple> // for std::tie
#include <deque>
#include <optional>

#include "../net_interface/net_interface.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"
#include "../postfile/outgoing_post.hpp"
#include "../constants/constants.hpp"

#include "read_response.hpp"
#include "sync_helpers.hpp"
#include "send_helpers.hpp"

template <typename post_request, typename delete_request, typename post_new_status, typename upload_attachments>
struct send_posts
{
public:
	unsigned int retries = 3;

	send_posts(post_request& post, delete_request& del, post_new_status& new_status, upload_attachments& upload) : post(post), del(del), new_status(new_status), upload(upload) { }

	void send(const std::string_view account, const std::string_view instanceurl, const std::string_view access_token)
	{
		retries = set_default(retries, 3, "Number of retries cannot be zero or less. Resetting to 3.\n", pl());

		const std::string baseurl = make_api_url(instanceurl, status_route);

		this->access_token = access_token;
		pl() << "Sending queued favorites for " << account << '\n';
		process_queue<queues::fav>(account, baseurl);

		pl() << "Sending queued boosts for " << account << '\n';
		process_queue<queues::boost>(account, baseurl);

		pl() << "Sending queued posts for " << account << '\n';
		const std::string mediaurl = make_api_url(instanceurl, media_route);
		process_posts(account, baseurl, mediaurl);
	}

private:
	std::string_view access_token;

	post_request& post;
	delete_request& del;
	post_new_status& new_status;
	upload_attachments& upload;


	template <queues toread>
	void process_queue(const std::string_view account, const std::string_view baseurl)
	{
		auto queuefile = get(toread, account);

		std::deque<std::string> failedids;

		while (!queuefile.parsed.empty())
		{
			std::string_view id = queuefile.parsed.front();

			const bool undo = should_undo(id);

			const std::string requesturl = paramaterize_url(baseurl, id, route<toread>(undo));

			pl() << "POST " << requesturl;
			const auto response = request_with_retries([&]() { return post(requesturl, access_token); }, retries, pl());
			if (response.success)
				pl() << " OK";
			else
				failedids.push_back(std::move(queuefile.parsed.front()));

			print_statistics(pl(), response.time_ms, response.tries);

			// remove ID from this queue
			queuefile.parsed.pop_front();
		}

		queuefile.parsed = std::move(failedids);
	}

	bool send_attachments(file_status_params& params, std::string_view mediaurl)
	{
		bool succeeded;
		std::string response;
		for (const auto& attachment : params.attachments)
		{
			if (!fs::exists(attachment.file))
			{
				pl() << "Could not find file: " << attachment.file << " skipping this post.\n";
				return false;
			}

			pl() << "Uploading " << attachment.file << ' ';

			auto request_response = request_with_retries([&]() { return upload(mediaurl, access_token, attachment.file, attachment.description); }, retries, pl());
			response = std::move(request_response.message);
			succeeded = request_response.success;

			print_statistics(pl(), request_response.time_ms, request_response.tries);
			if (succeeded)
			{
				params.attachment_ids.push_back(read_upload_id(response));
			}
			else
			{
				pl() << "Could not upload file. Skipping this post.";
				return false;
			}
		}

		return true;
	}

	void process_posts(const std::string_view account, const std::string_view statusurl, const std::string_view mediaurl)
	{
		auto queuefile = get(queues::post, account);

		std::deque<std::string> failed;

		const fs::path file_queue_directory = get_file_queue_directory(account);
		while (!queuefile.parsed.empty())
		{
			std::string_view id = queuefile.parsed.front();

			// this is either a file path OR a post ID with a minus sign at the end
			// if it has a minus sign at the end, assume it's an ID to be deleted.
			const bool undo = should_undo(id);

			bool succeeded = true;
			if (undo)
			{
				const std::string requesturl = paramaterize_url(statusurl, id, "");
				pl() << "DELETE " << requesturl;
				const auto response = request_with_retries([&]() { return del(requesturl, access_token); }, retries, pl());
				succeeded = response.success;
				if (succeeded)
					pl() << " OK";
				print_statistics(pl(), response.time_ms, response.tries);
			}
			else
			{
				const fs::path file_to_send = file_queue_directory / id;
				file_status_params params = read_params(file_to_send);

				if (!params.okay)
				{
					pl() << id << ": This post is a reply to a post that failed to send. Skipping.\n";
					succeeded = false;
				}

				if (succeeded)
				{
					succeeded = send_attachments(params, mediaurl);
				}

				std::string parsed_status_id;
				if (succeeded)
				{
					pl() << "Sending post: ";
					if (!params.content_warning.empty())
						pl() << "\nCW: " << params.content_warning << '\n';
					print_truncated_string(params.body, pl());
					pl() << '\n';

					auto request_response = request_with_retries([&]() { return new_status(statusurl, access_token, params); }, retries, pl());

					std::string response = std::move(request_response.message);
					succeeded = request_response.success;

					if (succeeded)
					{
						fs::remove(file_to_send);
						auto parsed_status = read_status(response);
						pl() << "Created post at " << parsed_status.url;
						parsed_status_id = std::move(parsed_status.id);
					}
					print_statistics(pl(), request_response.time_ms, request_response.tries);
				}

				if (!params.reply_id.empty())
				{
					// store the parsed status ID regardless- the empty string if this failed
					// or a real ID if it succeeded.
					store_thread_id(std::move(params.reply_id), std::move(parsed_status_id));
				}

				if (!succeeded && !params.reply_to.empty())
				{
					// if this one is a reply to another post in this queue
					// and that one succeeded but this one failed, then
					// we want to write the real post ID back to the file so it'll 
					// do the right thing next time we try to sync up

					outgoing_post post{ file_to_send };

					// if the reply ID in the file doesn't match the one we have
					// (because reply_to is now an ID to a post on the remote server)
					// replace it with the remote ID
					if (post.parsed.reply_to_id != params.reply_to)
					{
						post.parsed.reply_to_id = std::move(params.reply_to);
					}
				}
			}

			if (!succeeded)
				failed.push_back(std::move(queuefile.parsed.front()));

			// remove ID from this queue
			queuefile.parsed.pop_front();
		}

		queuefile.parsed = std::move(failed);
	}


	template <queues tosend>
	constexpr std::string_view route(bool undo) const
	{
		std::pair<std::string_view, std::string_view> toreturn;
		if constexpr (tosend == queues::fav)
		{
			toreturn = favroutepost;
		}

		if constexpr (tosend == queues::boost)
		{
			toreturn = boostroutepost;
		}

		return undo ? toreturn.second : toreturn.first;
	}

};

#endif
