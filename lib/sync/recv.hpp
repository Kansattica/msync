#ifndef _MSYNC_RECV_HPP_
#define _MSYNC_RECV_HPP_

#include <print_logger.hpp>

#include "../net_interface/net_interface.hpp"

#include "../options/user_options.hpp"
#include "../options/global_options.hpp"

#include "../constants/constants.hpp"
#include "../postlist/post_list.hpp"
#include "../util/util.hpp"

#include "sync_helpers.hpp"
#include "recv_helpers.hpp"

#include <string_view>
#include <algorithm>
#include <iterator>

template <typename get_posts>
struct recv_posts
{
public:
	unsigned int retries = 3;
	unsigned int max_requests = 10;
	unsigned int per_call = 20;

	recv_posts(get_posts& post_downloader) : download(post_downloader) {};

	void get(std::string_view account_name, user_options& account)
	{
		retries = set_default(retries, 3, "Number of retries cannot be zero or less. Resetting to 3.\n", pl());
		max_requests = set_default(max_requests, 5, "Maximum requests cannot be zero or less. Resetting to 5.\n", pl());
		per_call = set_default(per_call, 20, "Number of posts to get per call cannot be zero or less. Resetting to 20.\n", pl());

		pl() << "Downloading notifications for " << account_name << '\n';
		update_timeline<to_get::notifications, mastodon_notification>(account);
		pl() << "Downloading the home timeline for " << account_name << '\n';
	}

private:
	get_posts& download;

	template <to_get timeline, typename mastodon_entity>
	void update_timeline(user_options& account)
	{
		constexpr recv_parameters params = get_parameters<timeline>();

		const sync_settings sync_method = account.get_sync_option(params.sync_setting);

		if (sync_method == sync_settings::dont_sync)
			return;

		const std::string& access_token = account.get_option(user_option::access_token);

		const std::string_view last_recorded_id = get_or_empty(account.try_get_option(params.last_id_setting));

		const std::string url = make_api_url(account.get_option(user_option::instance_url), params.route);

		// if last_id isn't set, we just wanna get a bunch of posts from the server, newest first
		// otherwise, start getting posts either starting from or stopping at last_recorded_id, depending on the sync_method

		// if we sync oldest_first, make the last recorded ID the minimum ID and make new requests until we're caught up (no new posts)
		// if we sync newest_first, start getting the newest posts until we've reached the last recorded ID.

		// the other thing to keep in mind is that the newest posts are first back from the API (that is, the highest ID is at position 0)
		// but should be written to the file so that the newest post is at the bottom of the file, and so the lowest ID should be written first


		post_list<mastodon_entity> writer{ options().account_directory_location / params.filename };
		std::string highest_id;

		if (last_recorded_id == nullptr || sync_method == sync_settings::newest_first)
		{
			highest_id = newest_first(writer, url, access_token, last_recorded_id);
		}
		else if (sync_method == sync_settings::oldest_first)
		{
			highest_id = oldest_first(writer, url, access_token, last_recorded_id);
		}

		if (!highest_id.empty())
		{
			account.set_option(params.last_id_setting, std::move(highest_id));
		}
	}

	template <typename mastodon_entity>
	std::string newest_first(post_list<mastodon_entity>& writer, const std::string_view url, const std::string_view access_token, const std::string_view last_recorded_id)
	{
		std::string max_id;

		std::vector<mastodon_entity> total, incoming;
		timeline_params query_parameters;

		unsigned int i = 0;
		do
		{
			query_parameters.max_id = max_id;
			query_parameters.since_id = last_recorded_id;

			print_api_call(url, query_parameters, pl());

			auto response = request_with_retries([&]() { return download(url, access_token, query_parameters, per_call); }, retries);

			if (!response.first)
			{
				break;
			}

			incoming = deserialize<mastodon_entity>(response.second);

			max_id = lowest_id(incoming);

			total.insert(total.end(), std::make_move_iterator(incoming.begin()), std::make_move_iterator(incoming.end()));

		} while (++i < max_requests && !incoming.empty());

		plverb() << "Writing " << total.size() << " posts.\n";

		if (!total.empty())
		{
			// we want the latest post (highest ID) to be last, but it's in position 0, so iterate backwards
			std::for_each(total.rbegin(), total.rend(), [&writer](const auto& elem) { writer.write(elem); });
			return total.front().id;
		}

		return "";
	}

	template <typename mastodon_entity>
	std::string oldest_first(post_list<mastodon_entity>& writer, const std::string_view url, const std::string_view access_token, const std::string_view last_recorded_id)
	{
		std::vector<mastodon_entity> incoming;
		timeline_params query_parameters;
		query_parameters.min_id = last_recorded_id;
		
		std::string highest_id_seen;

		unsigned int i = 0;
		do
		{
			print_api_call(url, query_parameters, pl());

			auto response = request_with_retries([&]() { return download(url, access_token, query_parameters, per_call); }, retries);

			if (!response.first)
			{
				break;
			}

			incoming = deserialize<mastodon_entity>(response.second);

			if (!incoming.empty())
			{
				query_parameters.min_id = highest_id_seen = highest_id(incoming);

				plverb() << "Writing " << total.size() << " posts.\n";

				// we want the latest post (highest ID) to be last, but it's in position 0, so iterate backwards
				std::for_each(incoming.rbegin(), incoming.rend(), [&writer](const auto& elem) { writer.write(elem); });
			}

		} while (++i < max_requests && !incoming.empty());

		return highest_id_seen;
	}
};

#endif