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
#include "../options/user_options.hpp"
#include "../options/global_options.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"
#include "../postfile/outgoing_post.hpp"
#include "../constants/constants.hpp"

#include "read_response.hpp"
#include "sync_helpers.hpp"

template <typename post_request, typename delete_request, typename post_new_status, typename upload_attachments>
struct send_posts
{
public:
	int retries = 3;

	send_posts(post_request& post, delete_request& del, post_new_status& new_status, upload_attachments& upload) : post(post), del(del), new_status(new_status), upload(upload) { }

	void send_all()
	{
		options().foreach_account([this](const auto& user)
			{
				this->send(user.first, *user.second.get_option(user_option::instance_url), *user.second.get_option(user_option::access_token));
			});
		
	}

	void send(const std::string_view account, const std::string_view instanceurl, const std::string_view access_token)
	{
		if (retries < 1)
		{
			pl() << "Number of retries must be positive (got " << retries << "). Resetting to 3.\n";
			retries = 3;
		}

		const std::string baseurl = make_api_url(instanceurl, status_route());

		this->access_token = access_token;
		pl() << "Sending queued favorites for " << account << '\n';
		process_queue<queues::fav>(account, baseurl);

		pl() << "Sending queued boosts for " << account << '\n';
		process_queue<queues::boost>(account, baseurl);

		pl() << "Sending queued posts for " << account << '\n';
		const std::string mediaurl = make_api_url(instanceurl, media_route());
		process_posts(account, baseurl, mediaurl);
	}

private:
	std::string_view access_token;

	post_request& post;
	delete_request& del;
	post_new_status& new_status;
	upload_attachments& upload;

	constexpr const std::string_view status_route() const { return "/api/v1/statuses/"; }
	constexpr const std::string_view media_route() const { return "/api/v1/media/"; }

	constexpr const std::pair<std::string_view, std::string_view> favroutepost() const { return { "/favourite", "/unfavourite" }; }
	constexpr const std::pair<std::string_view, std::string_view> boostroutepost() const { return { "/reblog", "/unreblog" }; }

	template <queues toread>
	void process_queue(const std::string_view account, const std::string_view baseurl) 
	{
		auto queuefile = get(toread, account);

		std::deque<std::string> failedids;

		while (!queuefile.parsed.empty())
		{
			std::string_view id = queuefile.parsed.front();

			const bool undo = should_undo(id);

			const std::string requesturl = paramaterize_url(baseurl, id, undo ? route<toread, false>() : route<toread, true>());

			if (request_with_retries( [&]() { return post(requesturl, access_token); }).first)
				pl() << requesturl << " OK\n";
			else
				failedids.push_back(std::move(queuefile.parsed.front()));

			// remove ID from this queue
			queuefile.parsed.pop_front();
		}

		queuefile.parsed = std::move(failedids);
	}

	void print_status(const mastodon_status& status)
	{
		pl() << "Created post at " << status.url << '\n';

		if (!status.content_warning.empty())
			pl() << "CW: " << status.content_warning << '\n';

		static const int max_length = 30;
		std::string_view toprint{ status.content };
		if (toprint.size() > max_length)
		{
			toprint.remove_suffix(toprint.size() - max_length);
		}

		pl() << "Body: " << toprint;
		if (toprint.size() > max_length)
			pl() << "...";
		pl() << '\n';
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
				succeeded = request_with_retries([&]() { return del(requesturl, access_token); }).first;
				if (succeeded)
					pl() << "DELETE " << requesturl << " OK\n";
			}
			else
			{
				const fs::path file_to_send = file_queue_directory / id;
				file_status_params params = read_params(file_to_send);

				std::string response;
				for (const auto& attachment : params.attachments)
				{
					if (!fs::exists(attachment.file))
					{
						pl() << "Could not find file: " << attachment.file << " skipping this post.\n";
						succeeded = false;
						break;
					}

					std::tie(succeeded, response) = request_with_retries([&]() { return upload(mediaurl, access_token, attachment.file, attachment.description); });

					if (succeeded)
					{
						pl() << "Uploaded file: " << attachment.file << '\n';
						params.attachment_ids.push_back(read_upload_id(response));
					}
					else
					{
						pl() << "Could not upload file. Skipping this post.\n";
						break;
					}
				}

				if (succeeded)
				{
					status_params p{ std::move(params) };
					std::tie(succeeded, response) = request_with_retries([&]() { return new_status(statusurl, access_token, p); });
					if (succeeded)
					{
						fs::remove(file_to_send);
						auto parsed_status = read_status(response);
						if (!params.reply_id.empty())
						{
							store_thread_id(std::move(params.reply_id), std::move(parsed_status.id));
						}
						print_status(parsed_status);
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


	template <queues tosend, bool create>
	constexpr std::string_view route() const
	{
		std::pair<std::string_view, std::string_view> toreturn;
		if constexpr (tosend == queues::fav)
		{
			toreturn = favroutepost();
		}

		if constexpr (tosend == queues::boost)
		{
			toreturn = boostroutepost();
		}

		return std::get<create ? 0 : 1>(toreturn);
	}

	template <typename make_request>
	std::pair<bool, std::string> request_with_retries(make_request req)
	{
		for (int i = 0; i < retries; i++)
		{
			const net_response response = req();

			pl() << get_error_message(response.status_code, verbose_logs);

			// later, handle what happens if we get rate limited

			if (response.retryable_error)
			{
				// should retry
				continue;
			}

			// some other error, assume unrecoverable
			if (!response.okay)
			{
				pl() << response.message << '\n';
				return std::make_pair(false, std::move(response.message));
			}

			// must be 200, OK response
			return std::make_pair(true, std::move(response.message));
		}
		return std::make_pair(false, "Maximum retries reached.");
	}

};

#endif
