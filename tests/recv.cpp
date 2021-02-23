#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#include <catch2/catch.hpp>

#include "../lib/sync/recv.hpp"
#include "../lib/options/global_options.hpp"

#include "test_helpers.hpp"
#include "mock_network.hpp"
#include "sync_test_common.hpp"
#include "../lib/netinterface/net_interface.hpp"

#include "../lib/constants/constants.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <tuple>
#include <iomanip>
#include <chrono>
#include <sstream>

using namespace std::string_view_literals;

constexpr unsigned int lowest_post_id = 1000000;
constexpr unsigned int lowest_notif_id = 10000;
constexpr unsigned int lowest_bookmark_id = 2000000;

struct mock_network_get : public mock_network
{
	std::vector<get_mock_args> arguments;

	unsigned int total_post_count = 310;
	unsigned int total_bookmark_count = 220;
	unsigned int total_notif_count = 240;

	bool should_rate_limit = false;
	std::chrono::seconds rate_limit_wait = std::chrono::seconds(20);
	
	net_response operator()(std::string_view url, std::string_view access_token, const timeline_params& params, unsigned int limit)
	{
		arguments.push_back(get_mock_args{{0, std::string{url}, std::string{access_token}},
			std::string{params.min_id}, std::string{params.max_id}, std::string{params.since_id}, copy_excludes(params.exclude_notifs), limit });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		if (!toreturn.okay)
		{
			if (toreturn.retryable_error && should_rate_limit)
			{
				toreturn.status_code = 429;

				const auto wait_until = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + rate_limit_wait);
				struct tm wait_until_struct {};
				wrap_gmtime(&wait_until_struct, &wait_until);

				std::ostringstream timestamp;
				timestamp << std::put_time(&wait_until_struct, "%FT%T.12312Z");
				toreturn.message = timestamp.str();
				return toreturn;
			}

			toreturn.message = R"({ "error": "some problem" })";
			return toreturn;
		}

		
		// if the url ends in "notifications" do notifications. if it ends in "home", do statuses and so on
		const auto [json_func, lowest_id, total_count] = [url, this]() {
			std::string_view url_view = url.substr(url.find_last_of('/') + 1);
			if (url_view == "notifications") { return std::make_tuple(make_notification_json, lowest_notif_id, total_notif_count); }
			if (url_view == "home") { return std::make_tuple(make_status_json, lowest_post_id, total_post_count); }
			if (url_view == "bookmarks") { return std::make_tuple(make_status_json, lowest_bookmark_id, total_bookmark_count); }

			CAPTURE(url);
			FAIL("Hey, I don't know what to do with this URL.");

			// some compilers complain if control could reach the end of the function (even though it almost certainly won't here- I'm pretty sure FAIL throws an exception)
			return std::make_tuple(make_status_json, 0u, 0u);
		}();

		auto upper_bound = lowest_id + total_count;
		auto lower_bound = upper_bound - limit;

		if (!params.max_id.empty())
		{
			std::from_chars(params.max_id.data(), params.max_id.data() + params.max_id.size(), upper_bound);
			upper_bound--; //don't return the post with the id that equals max_id
			lower_bound = upper_bound - limit;
		}

		if (!params.min_id.empty())
		{
			std::from_chars(params.min_id.data(), params.min_id.data() + params.min_id.size(), lower_bound);
			lower_bound++;
			upper_bound = std::min(upper_bound, lower_bound + limit);
		}

		if (!params.since_id.empty())
		{
			unsigned int since;
			std::from_chars(params.since_id.data(), params.since_id.data() + params.since_id.size(), since);
			since++;

			// don't return any statuses as old or older than since
			lower_bound = std::max(lower_bound, since);
		}

		if (lower_bound < lowest_id) 
		{ 
			lower_bound = lowest_id;
		}

		if (upper_bound > lowest_id + total_post_count)
		{
			upper_bound = lowest_id + total_post_count;
		}

		if (lower_bound >= upper_bound)
		{
			toreturn.message = "[]";
			return toreturn;
		}

		REQUIRE((upper_bound - lower_bound) <= limit);
		toreturn.message = make_json_array(json_func, lower_bound, upper_bound);

		return toreturn;
	}
};

SCENARIO("Recv downloads and writes the correct number of posts.")
{
	logs_off = true;

	static constexpr std::string_view account_name = "user@crime.egg";
	static constexpr std::string_view expected_notification_endpoint = "https://crime.egg/api/v1/notifications";
	static constexpr std::string_view expected_bookmark_endpoint = "https://crime.egg/api/v1/bookmarks";
	static constexpr std::string_view expected_home_endpoint = "https://crime.egg/api/v1/timelines/home";
	static constexpr std::string_view expected_access_token = "token!";

	const test_dir account_dir = temporary_directory();

	global_options options{ account_dir.dirname };

	auto& account = options.add_new_account(std::string{ account_name });

	const auto user_dir = account_dir.dirname / account.first;
	const auto home_timeline_file = user_dir / Home_Timeline_Filename;
	const auto notifications_file = user_dir / Notifications_Filename;
	const auto bookmarks_file = user_dir / Bookmarks_Filename;

	REQUIRE(account.first == account_name);

	REQUIRE(account.second.get_user_directory().filename().string() == account_name);

	account.second.set_option(user_option::account_name, "user");
	account.second.set_option(user_option::instance_url, "crime.egg");
	account.second.set_option(user_option::access_token, "token!");

	mock_network_get mock_get;

	GIVEN("A user account with no previously stored information.")
	{
		WHEN("That account is given to recv and told to update.")
		{
			recv_posts post_getter{ mock_get };

			post_getter.get(account.second);

			THEN("Five calls each were made to the home, notification, and bookmark API endpoints with the correct URLs, default limits, and access tokens.")
			{
				const auto& args = mock_get.arguments;
				REQUIRE(args.size() == 15);
				REQUIRE(std::all_of(args.begin(), args.begin() + 5, [&](const get_mock_args& arg) { return arg.url == expected_notification_endpoint && arg.limit == 30; }));
				REQUIRE(std::all_of(args.begin() + 5, args.begin() + 10, [&](const get_mock_args& arg) { return arg.url == expected_home_endpoint && arg.limit == 40; }));
				REQUIRE(std::all_of(args.begin() + 10, args.end(), [&](const get_mock_args& arg) { return arg.url == expected_bookmark_endpoint && arg.limit == 40; }));
				REQUIRE(std::all_of(args.begin(), args.end(), [&](const get_mock_args& arg) { return arg.access_token == expected_access_token && arg.exclude_notifs.empty(); }));
			}

			THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
			{
				constexpr int expected_home_statuses = 40 * 5;
				constexpr int expected_notifications = 30 * 5;
				constexpr int expected_bookmark_statuses = 40 * 5;

				verify_file(home_timeline_file, expected_home_statuses, "status id: ");
				verify_file(notifications_file, expected_notifications, "notification id: ");
				verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
			}

			THEN("The correct last IDs are saved back to the account.")
			{
				std::array<char, 10> id_char_buf;

				REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
				REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
				REQUIRE(account.second.get_option(user_option::last_bookmark_id) == sv_to_chars(lowest_bookmark_id + mock_get.total_bookmark_count, id_char_buf));
			}

			AND_WHEN("More posts, notifications, and bookmarks are added and get is called again.")
			{
				mock_get.arguments.clear();
				mock_get.total_post_count += 10;
				mock_get.total_notif_count += 15;
				mock_get.total_bookmark_count += 5;

				post_getter.get(account.second);

				THEN("Only one call was made to each endpoint.")
				{
					const auto& args = mock_get.arguments;
					REQUIRE(args.size() == 3);
					REQUIRE(args[0].url == expected_notification_endpoint);
					REQUIRE(args[0].limit == 30);
					REQUIRE(args[1].url == expected_home_endpoint);
					REQUIRE(args[1].limit == 40);
					REQUIRE(args[2].url == expected_bookmark_endpoint);
					REQUIRE(args[2].limit == 40);
				}

				THEN("The correct last IDs are saved back to the account.")
				{
					std::array<char, 10> id_char_buf;

					REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_bookmark_id) == sv_to_chars(lowest_bookmark_id + mock_get.total_bookmark_count, id_char_buf));
				}

				THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
				{
					// the -1 is because adding 10 to the post count only adds 9 new statuses because you already got the 0th status last time
					// this feels weird because of the weird mix of half-open ranges and fully closed ranges, but I believe it's correct
					constexpr int expected_home_statuses = 40 * 5 + 10 - 1;
					constexpr int expected_notifications = 30 * 5 + 15 - 1;
					constexpr int expected_bookmark_statuses = 40 * 5 + 5 - 1;

					verify_file(home_timeline_file, expected_home_statuses, "status id: ");
					verify_file(notifications_file, expected_notifications, "notification id: ");
					verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
				}
			}

			AND_WHEN("More posts, notifications, and bookmarks are added and get is called again, but we're rate limited.")
			{
				mock_get.arguments.clear();
				mock_get.total_post_count += 10;
				mock_get.total_notif_count += 15;
				mock_get.total_bookmark_count += 5;

				mock_get.should_rate_limit = true;
				mock_get.set_succeed_after(2);

				const auto start_time = std::chrono::system_clock::now();
				post_getter.get(account.second);

				// this is a little weird
				// because Catch runs everything leading up to the test case for each THEN block,
				// having everything in separate blocks means the test gets run that many times. 
				// Normally, this is good, expected, and desirable, but for the rate limiting, it means you wait a long time for no reason.
				// so, all the test cases get combined into one instead of waiting for nothing.
				THEN("The appropriate amount of time was waited after a rate-limited call and everything is as it should be.")
				{
					// cut it a second of slack because of clock jitter and stuff
					const auto target_time = start_time + (2 * mock_get.rate_limit_wait) - std::chrono::seconds(1);
					REQUIRE(std::chrono::system_clock::now() >= target_time);

				//THEN("Two calls were made to each endpoint.")
				{
					const auto& args = mock_get.arguments;
					REQUIRE(args.size() == 6);
					REQUIRE(args[0].url == expected_notification_endpoint);
					REQUIRE(args[0].limit == 30);
					REQUIRE(args[1].url == expected_notification_endpoint);
					REQUIRE(args[1].limit == 30);
					REQUIRE(args[2].url == expected_home_endpoint);
					REQUIRE(args[2].limit == 40);
					REQUIRE(args[3].url == expected_home_endpoint);
					REQUIRE(args[3].limit == 40);
					REQUIRE(args[4].url == expected_bookmark_endpoint);
					REQUIRE(args[4].limit == 40);
					REQUIRE(args[5].url == expected_bookmark_endpoint);
					REQUIRE(args[5].limit == 40);
				}

				//THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
				{
					// the -1 is because adding 10 to the post count only adds 9 new statuses because you already got the 0th status last time
					// this feels weird because of the weird mix of half-open ranges and fully closed ranges, but I believe it's correct
					constexpr int expected_home_statuses = 40 * 5 + 10 - 1;
					constexpr int expected_notifications = 30 * 5 + 15 - 1;
					constexpr int expected_bookmark_statuses = 40 * 5 + 5 - 1;

					verify_file(home_timeline_file, expected_home_statuses, "status id: ");
					verify_file(notifications_file, expected_notifications, "notification id: ");
					verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
				}


				}
			}

			AND_WHEN("More posts, notifications, and bookmarks are added and get is called again, but on a flaky connection.")
			{
				mock_get.arguments.clear();
				mock_get.total_post_count += 10;
				mock_get.total_notif_count += 15;
				mock_get.total_bookmark_count += 5;

				mock_get.set_succeed_after(2);

				post_getter.get(account.second);

				THEN("Two calls were made to each endpoint.")
				{
					const auto& args = mock_get.arguments;
					REQUIRE(args.size() == 6);
					REQUIRE(args[0].url == expected_notification_endpoint);
					REQUIRE(args[0].limit == 30);
					REQUIRE(args[1].url == expected_notification_endpoint);
					REQUIRE(args[1].limit == 30);
					REQUIRE(args[2].url == expected_home_endpoint);
					REQUIRE(args[2].limit == 40);
					REQUIRE(args[3].url == expected_home_endpoint);
					REQUIRE(args[3].limit == 40);
					REQUIRE(args[4].url == expected_bookmark_endpoint);
					REQUIRE(args[4].limit == 40);
					REQUIRE(args[5].url == expected_bookmark_endpoint);
					REQUIRE(args[5].limit == 40);
				}

				THEN("The correct last IDs are saved back to the account.")
				{
					std::array<char, 10> id_char_buf;

					REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_bookmark_id) == sv_to_chars(lowest_bookmark_id + mock_get.total_bookmark_count, id_char_buf));
				}

				THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
				{
					// the -1 is because adding 10 to the post count only adds 9 new statuses because you already got the 0th status last time
					// this feels weird because of the weird mix of half-open ranges and fully closed ranges, but I believe it's correct
					constexpr int expected_home_statuses = 40 * 5 + 10 - 1;
					constexpr int expected_notifications = 30 * 5 + 15 - 1;
					constexpr int expected_bookmark_statuses = 40 * 5 + 5 - 1;

					verify_file(home_timeline_file, expected_home_statuses, "status id: ");
					verify_file(notifications_file, expected_notifications, "notification id: ");
					verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
				}
			}
		}
	}

	GIVEN("A user account that excludes some notification types.")
	{
		account.second.set_bool_option(user_option::exclude_boosts, true);
		account.second.set_bool_option(user_option::exclude_follows, true);
		account.second.set_bool_option(user_option::exclude_favs, false);

		WHEN("That account is given to recv and told to update.")
		{
			recv_posts post_getter{ mock_get };

			post_getter.get(account.second);

			THEN("Calls to the notification API correctly include the excluded notif types.")
			{
				const auto& args = mock_get.arguments;
				for (auto it = args.begin(); it != args.begin() + 5; ++it)
				{
					REQUIRE_THAT(it->exclude_notifs, Catch::UnorderedEquals(std::vector<std::string>{ "reblog", "follow" }));
				}
			}

			THEN("Five calls each were made to the home, notification, and bookmark API endpoints with the correct URLs, default limits, and access tokens.")
			{
				const auto& args = mock_get.arguments;
				CAPTURE(args);
				REQUIRE(args.size() == 15);
				REQUIRE(std::all_of(args.begin(), args.begin() + 5, [&](const get_mock_args& arg) { return arg.url == expected_notification_endpoint && arg.limit == 30; }));
				REQUIRE(std::all_of(args.begin() + 5, args.begin() + 10, [&](const get_mock_args& arg) { return arg.url == expected_home_endpoint && arg.limit == 40 && arg.exclude_notifs.empty(); }));
				REQUIRE(std::all_of(args.begin() + 10, args.end(), [&](const get_mock_args& arg) { return arg.url == expected_bookmark_endpoint && arg.limit == 40 && arg.exclude_notifs.empty(); }));
				REQUIRE(std::all_of(args.begin(), args.end(), [&](const get_mock_args& arg) { return arg.access_token == expected_access_token; }));
			}

			THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
			{
				constexpr int expected_home_statuses = 40 * 5;
				constexpr int expected_notifications = 30 * 5;
				constexpr int expected_bookmark_statuses = 40 * 5;

				verify_file(home_timeline_file, expected_home_statuses, "status id: ");
				verify_file(notifications_file, expected_notifications, "notification id: ");
				verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
			}

			THEN("The correct last IDs are saved back to the account.")
			{
				std::array<char, 10> id_char_buf;

				REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
				REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
				REQUIRE(account.second.get_option(user_option::last_bookmark_id) == sv_to_chars(lowest_bookmark_id + mock_get.total_bookmark_count, id_char_buf));
			}

			AND_WHEN("More posts, notifications, and bookmarks are added and get is called again.")
			{
				mock_get.arguments.clear();
				mock_get.total_post_count += 10;
				mock_get.total_notif_count += 15;
				mock_get.total_bookmark_count += 5;

				post_getter.get(account.second);

				THEN("Only one call was made to each endpoint.")
				{
					const auto& args = mock_get.arguments;
					REQUIRE(args.size() == 3);
					REQUIRE(args[0].url == expected_notification_endpoint);
					REQUIRE(args[0].limit == 30);
					REQUIRE(args[1].url == expected_home_endpoint);
					REQUIRE(args[1].limit == 40);
					REQUIRE(args[2].url == expected_bookmark_endpoint);
					REQUIRE(args[2].limit == 40);
				}

				THEN("The correct last IDs are saved back to the account.")
				{
					std::array<char, 10> id_char_buf;

					REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
					REQUIRE(account.second.get_option(user_option::last_bookmark_id) == sv_to_chars(lowest_bookmark_id + mock_get.total_bookmark_count, id_char_buf));
				}

				THEN("All three files have the expected number of posts, and the IDs are strictly increasing.")
				{
					// the -1 is because adding 10 to the post count only adds 9 new statuses because you already got the 0th status last time
					// this feels weird because of the weird mix of half-open ranges and fully closed ranges, but I believe it's correct
					constexpr int expected_home_statuses = 40 * 5 + 10 - 1;
					constexpr int expected_notifications = 30 * 5 + 15 - 1;
					constexpr int expected_bookmark_statuses = 40 * 5 + 5 - 1;

					verify_file(home_timeline_file, expected_home_statuses, "status id: ");
					verify_file(notifications_file, expected_notifications, "notification id: ");
					verify_file(bookmarks_file, expected_bookmark_statuses, "status id: ");
				}
			}
		}
	}
}
