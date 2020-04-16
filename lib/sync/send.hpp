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
#include <unordered_map>

#include "../netinterface/net_interface.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"
#include "../postfile/outgoing_post.hpp"
#include "../constants/constants.hpp"

#include "read_response.hpp"
#include "sync_helpers.hpp"
#include "send_helpers.hpp"
#include "lazy_url_builder.hpp"

template <typename post_request, typename delete_request, typename post_new_status, typename upload_attachments>
struct send_posts
{
public:
	unsigned int retries = 3;

	send_posts(post_request& post, delete_request& del, post_new_status& new_status, upload_attachments& upload) : post(post), del(del), new_status(new_status), upload(upload) { }

	void send(const std::string& account, const std::string_view instance_url, const std::string_view access_token)
	{
		retries = set_default(retries, 3, "Number of retries cannot be zero or less. Resetting to 3.\n", pl());

		process_queue(account, instance_url, access_token);
	}


private:
	post_request& post;
	delete_request& del;
	post_new_status& new_status;
	upload_attachments& upload;

	bool make_api_call(const api_call& to_make, lazy_url_builder urls, const std::string& account, std::string_view access_token)
	{
		switch (to_make.queued_call)
		{
		case api_route::fav:
		case api_route::unfav:
		case api_route::boost:
		case api_route::unboost:
			return simple_call(post, "POST", retries, paramaterize_url(urls.status_url(), to_make.argument, ROUTE_LOOKUP[static_cast<uint8_t>(to_make.queued_call)]), access_token);
		case api_route::post:
			// posts are a little trickier
			return send_post(account, access_token, urls.status_url(), urls.media_url(), to_make.argument);
		case api_route::unpost:
			return simple_call(del, "DELETE", retries, paramaterize_url(urls.status_url(), to_make.argument, ROUTE_LOOKUP[static_cast<uint8_t>(to_make.queued_call)]), access_token);
		default:
			return false;
		}
	}

	void process_queue(const std::string& account, const std::string_view instance_url, const std::string_view access_token)
	{
		auto queuelist = get(account);

		std::deque<api_call> failed;

		lazy_url_builder urls(instance_url);

		while (!queuelist.parsed.empty())
		{
			if (!make_api_call(queuelist.parsed.front(), instance_url, account, access_token))
				failed.push_back(std::move(queuelist.parsed.front()));
			queuelist.parsed.pop_front();
		}

		queuelist.parsed = std::move(failed);
	}

	bool send_attachments(file_status_params& params, const std::string& mediaurl, std::string_view access_token)
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

	const fs::path& get_cached_file_queue_dir(const std::string& account)
	{
		static std::unordered_map<std::string_view, fs::path> file_queue_dir_cache;

		auto found = file_queue_dir_cache.find(account);

		if (found != file_queue_dir_cache.end())
			return found->second;

		return (file_queue_dir_cache.insert({ account, get_file_queue_directory(account) })).first->second;
	}

	bool send_post(const std::string& account, const std::string_view access_token, const std::string& statusurl, const std::string& mediaurl, const std::string& post_filename)
	{
		const fs::path file_to_send = get_cached_file_queue_dir(account) / post_filename;

		file_status_params params = read_params(file_to_send);

		bool succeeded = true;
		if (!params.okay)
		{
			pl() << post_filename << ": This post is a reply to a post that failed to send. Skipping.\n";
			succeeded = false;
		}

		if (succeeded)
		{
			succeeded = send_attachments(params, mediaurl, access_token);
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

		return succeeded;
	}
};

#endif
