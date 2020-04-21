#ifndef _MSYNC_RECV_HPP_
#define _MSYNC_RECV_HPP_

#include <print_logger.hpp>

#include "../netinterface/net_interface.hpp"
#include "../accountdirectory/account_directory.hpp"

#include "../options/user_options.hpp"

#include "../postlist/post_list.hpp"
#include "../util/util.hpp"

#include "sync_helpers.hpp"
#include "recv_helpers.hpp"

#include <filesystem.hpp>
#include <string_view>
#include <algorithm>
#include <iterator>
#include <limits>
#include <array>
#include <utility>

template <typename get_posts>
struct recv_posts
{
public:
	unsigned int retries = 3;
	unsigned int max_requests = 0;
	unsigned int per_call = 0;

	recv_posts(get_posts& post_downloader) : download(post_downloader) {};

	void get(user_options& account)
	{
		retries = set_default(retries, 3, "Number of retries cannot be zero or less. Resetting to 3.\n", pl());

		// basically, Mastodon will only return, at most, twice its default limit
		// which, as of this writing, means the maximum is 40 for statuses and 30 for notifications
		// (the documentation lies and says that the limit is the same for both)

		exclude_notif_types = make_excludes(account);

		// We want the last segment of the path. std::filesystem doesn't provide a function for this (filename doesn't work because this is a directory, stem lops off everything after the last .
		// so, get an iterator to past-the-end, back it up one, then get the string version of it.
		const std::string account_name = (--account.get_user_directory().end())->string();
		pl() << "Downloading notifications for " << account_name << '\n';
		update_timeline<to_get::notifications, mastodon_notification, true>(account, account.get_user_directory(), clamp_or_default(per_call, 30));

		pl() << "Downloading the home timeline for " << account_name << '\n';
		update_timeline<to_get::home, mastodon_status>(account, account.get_user_directory(), clamp_or_default(per_call, 40));
	}

private:
	get_posts& download;
	std::vector<std::string_view> exclude_notif_types;

	template <to_get timeline, typename mastodon_entity, bool use_excludes = false>
	void update_timeline(user_options& account, const fs::path& user_folder, unsigned int limit)
	{
		const CONSTEXPR_IF_NOT_BOOST recv_parameters params = get_parameters<timeline>();

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

		const fs::path target_file = user_folder / params.filename;
		plverb() << "Writing to " << target_file << '\n';

		post_list<mastodon_entity> writer{ target_file };
		std::string highest_id;

		if (last_recorded_id.empty() || sync_method == sync_settings::newest_first)
		{
			highest_id = newest_first<mastodon_entity, use_excludes>(writer, url, access_token, last_recorded_id, limit);
		}
		else if (sync_method == sync_settings::oldest_first) //else if because dont_sync is an option
		{
			highest_id = oldest_first<mastodon_entity, use_excludes>(writer, url, access_token, last_recorded_id, limit);
		}

		if (!highest_id.empty())
		{
			account.set_option(params.last_id_setting, std::move(highest_id));
		}
	}

	template <typename mastodon_entity, bool use_excludes>
	std::string newest_first(post_list<mastodon_entity>& writer, const std::string_view url, const std::string_view access_token, const std::string_view last_recorded_id, unsigned int limit)
	{
		std::string max_id;

		std::vector<mastodon_entity> total, incoming;

		timeline_params query_parameters;
		query_parameters.since_id = last_recorded_id;

		if constexpr (use_excludes) { query_parameters.exclude_notifs = &exclude_notif_types; }

		// if max_requests is zero, that means "make calls until caught up"
		// however, if we don't have a last recorded ID, make five requests instead so we don't get all posts from now to the beginning of time

		unsigned int loop_iterations = max_requests;
		if (loop_iterations == 0)
			loop_iterations = last_recorded_id.empty() ? 5 : std::numeric_limits<unsigned int>::max();

		do
		{
			query_parameters.max_id = max_id;

			print_api_call(url, limit, query_parameters, pl());

			auto response = request_with_retries([&]() { return download(url, access_token, query_parameters, limit); }, retries, pl());

			print_statistics(pl(), response.time_ms, response.tries);

			if (!response.success)
			{
				break;
			}

			incoming = deserialize<mastodon_entity>(response.message);

			plverb() << "Downloaded " << incoming.size() << " posts.\n";

			if (!incoming.empty())
			{
				// can only call lowest_id on a non-empty vector
				max_id = lowest_id(incoming);
				total.insert(total.end(), std::make_move_iterator(incoming.begin()), std::make_move_iterator(incoming.end()));
			}

			loop_iterations--;

			// if you get less than you asked for, you're done
		} while (loop_iterations > 0 && (incoming.size() == limit));

		plverb() << "Writing " << total.size() << " posts.\n";

		if (!total.empty())
		{
			// we want the latest post (highest ID) to be last, but it's in position 0, so iterate backwards
			std::for_each(total.rbegin(), total.rend(), [&writer](const auto& elem) { writer.write(elem); });
			return total.front().id;
		}

		return "";
	}

	template <typename mastodon_entity, bool use_excludes>
	std::string oldest_first(post_list<mastodon_entity>& writer, const std::string_view url, const std::string_view access_token, const std::string_view last_recorded_id, unsigned int limit)
	{
		std::vector<mastodon_entity> incoming;

		timeline_params query_parameters;
		query_parameters.min_id = last_recorded_id;

		if constexpr (use_excludes) { query_parameters.exclude_notifs = &exclude_notif_types; }

		std::string highest_id_seen;

		// max_requests being zero means "request until caught up".
		// oldest_first doesn't have to worry about that "first time" weirdness, the first download will always use newest_first
		unsigned int loop_iterations = max_requests;
		if (loop_iterations == 0)
			loop_iterations = std::numeric_limits<unsigned int>::max();
		do
		{
			print_api_call(url, limit, query_parameters, pl());

			const auto response = request_with_retries([&]() { return download(url, access_token, query_parameters, limit); }, retries, pl());

			print_statistics(pl(), response.time_ms, response.tries);

			if (!response.success)
			{
				break;
			}

			incoming = deserialize<mastodon_entity>(response.message);

			plverb() << "Writing " << incoming.size() << " posts.\n";

			if (!incoming.empty())
			{
				query_parameters.min_id = highest_id_seen = highest_id(incoming);

				// we want the latest post (highest ID) to be last, but it's in position 0, so iterate backwards
				std::for_each(incoming.rbegin(), incoming.rend(), [&writer](const auto& elem) { writer.write(elem); });
			}

			--loop_iterations;

			// if you get less than you asked for, you're done
		} while (loop_iterations > 0 && (incoming.size() == limit));

		return highest_id_seen;
	}
};

#endif
