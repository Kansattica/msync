#ifndef _MSYNC_RECV_HPP_
#define _MSYNC_RECV_HPP_

#include <print_logger.hpp>

#include "../net_interface/net_interface.hpp"

#include "../options/user_options.hpp"

#include "../constants/constants.hpp"
#include "../postlist/post_list.hpp"
#include "../util/util.hpp"

#include "sync_helpers.hpp"

#include <string_view>

enum class to_get { notifications, home, lists };

struct timeline_parameters { user_option last_id_setting; user_option sync_setting; std::string_view route; std::string_view filename; };

template <to_get timeline>
constexpr timeline_parameters get_parameters()
{
	if constexpr (timeline == to_get::notifications)
	{
		return { user_option::last_notification_id, user_option::pull_notifications, notifications_route, Notifications_Filename };
	}
}

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
		update_timeline<to_get::notifications>(account);
		pl() << "Downloading the home timeline for " << account_name << '\n';
	}

private:
	get_posts& download;


	template <to_get timeline>
	void update_timeline(user_options& account)
	{
		constexpr auto params = get_parameters<timeline>();

		const sync_settings sync_method = account.get_sync_option(params.sync_setting);

		if (sync_method == sync_settings::dont_sync)
			return;

		const std::string& access_token = account.get_option(user_option::access_token);
		const std::string* last_id = account.try_get_option(params.last_id_setting);

		const std::string url = make_api_url(account.get_option(user_option::instance_url), params.route);

		// if last_id isn't set, we just wanna get a bunch of posts from the server, newest first
		// otherwise, start getting posts either starting from or stopping at last_id, depending on the sync_method

		for (unsigned int i = 0; i < max_requests; i++)
		{
			auto posts = download(url, access_token, "", per_call);
		}

	}
};

#endif