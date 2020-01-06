#include <catch2/catch.hpp>

#include "../lib/sync/recv.hpp"
#include "../lib/options/global_options.hpp"

#include "test_helpers.hpp"
#include "mock_network.hpp"
#include "../lib/net_interface/net_interface.hpp"

#include "../lib/constants/constants.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <charconv>
#include <system_error>
#include <tuple>

using namespace std::string_view_literals;

template <typename Number>
std::string_view sv_to_chars(Number n, std::array<char, 10>& char_buf)
{
	// see https://en.cppreference.com/w/cpp/utility/to_chars
	// this avoids an allocation compared to std::to_string

	// note that this function takes a character buffer that it will clobber and returns a string view into it
	// this is to avoid allocations and also not return pointers into memory that will be freed when the function returns.

	const auto [end, err] = std::to_chars(char_buf.data(), char_buf.data() + char_buf.size(), n);
	if (err != std::errc()) { FAIL("You messed up with to_chars, ya dingus."); }
	return std::string_view(char_buf.data(), end - char_buf.data());
}

template <typename make_object>
std::string make_json_array(make_object func, unsigned int min_id, unsigned int max_id)
{
	std::array<char, 10> char_buf;

	std::string toreturn(1, '[');

	// basically, it shouldn't return max_id or min_id itself
	// and the newest (highest ID) goes first
	for (unsigned int id = max_id; id > min_id; id--)
	{
		func(sv_to_chars(id, char_buf), toreturn);
		toreturn.append(1, ',');
	}
	toreturn.pop_back(); //get rid of that last comma
	if (!toreturn.empty())
		toreturn += ']';
	return toreturn;
}

struct get_mock_args : basic_mock_args
{
	std::string min_id;
	std::string max_id;
	std::string since_id;

	unsigned int limit;
};

constexpr unsigned int lowest_post_id = 1000000;
constexpr unsigned int lowest_notif_id = 10000;

struct mock_network_get : public mock_network
{
	std::vector<get_mock_args> arguments;

	unsigned int total_post_count = 310;
	unsigned int total_notif_count = 240;

	net_response operator()(std::string_view url, std::string_view access_token, const timeline_params& params, unsigned int limit)
	{
		arguments.push_back(get_mock_args{ std::string{url}, std::string{access_token}, 
			std::string{params.min_id}, std::string{params.max_id}, std::string{params.since_id}, limit });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		if (!toreturn.okay)
		{
			toreturn.message = R"({ "error": "some problem" })";
			return toreturn;
		}
		
		// if the url ends in "notifications" do notifications. if it ends in "home", do statuses
		const auto [json_func, lowest_id, total_count] = [url, this]() {
			std::string_view url_view = url.substr(url.find_last_of('/') + 1);
			if (url_view == "notifications") { return std::make_tuple(make_notification_json, lowest_notif_id, total_notif_count); }
			if (url_view == "home") { return std::make_tuple(make_status_json, lowest_post_id, total_post_count); }

			CAPTURE(url);
			FAIL("Hey, I don't know what to do with this URL.");
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

void verify_file(const fs::path& file, int expected_count, const std::string& id_starts_with)
{
	constexpr std::string_view dashes = "--------------";

	const auto lines = read_lines(file);

	bool read_next = true;
	unsigned int last_id = 0;
	unsigned int total = 0;

	CAPTURE(file);

	for (const auto& line : lines)
	{
		if (line == dashes)
		{
			read_next = true;
			continue;
		}

		if (read_next)
		{
			read_next = false;
			REQUIRE_THAT(line, Catch::StartsWith(id_starts_with));

			unsigned int this_id;
			std::from_chars(line.data() + id_starts_with.size(), line.data() + line.size(), this_id);

			REQUIRE(this_id > last_id);

			last_id = this_id;
			total++;
		}
	}

	REQUIRE(expected_count == total);
}

SCENARIO("Recv downloads and writes the correct number of posts.")
{
	constexpr std::string_view account_name = "user@crime.egg";
	const test_file account_dir = account_directory();

	const static auto user_dir = account_dir.filename / account_name;
	const static auto home_timeline_file = user_dir / Home_Timeline_Filename;
	const static auto notifications_file = user_dir / Notifications_Filename;

	auto& account = options().add_new_account(std::string{ account_name });

	REQUIRE(account.first == account_name);

	account.second.set_option(user_option::account_name, "user");
	account.second.set_option(user_option::instance_url, "crime.egg");
	account.second.set_option(user_option::access_token, "token!");

	mock_network_get mock_get;

	GIVEN("A user account with no previously stored information.")
	{
		WHEN("That account is given to recv and told to update.")
		{
			recv_posts post_getter{ mock_get };

			post_getter.get(account.first, account.second);

			THEN("Five calls each were made to the home and notification API endpoints with the correct URLs and access tokens.")
			{
				const auto& args = mock_get.arguments;
				CAPTURE(args);
				REQUIRE(args.size() == 10);
				REQUIRE(std::all_of(args.begin(), args.begin() + 5, [](const get_mock_args& arg) { return arg.url == "https://crime.egg/api/v1/notifications"sv; }));
				REQUIRE(std::all_of(args.begin() + 5, args.end(), [](const get_mock_args& arg) { return arg.url == "https://crime.egg/api/v1/timelines/home"sv; }));
				REQUIRE(std::all_of(args.begin(), args.end(), [](const get_mock_args& arg) { return arg.access_token == "token!"sv; }));
			}

			constexpr int expected_home_statuses = 40 * 5;
			constexpr int expected_notifications = 30 * 5;

			THEN("Both files have the expected number of posts, and the IDs are strictly increasing.")
			{
				verify_file(home_timeline_file, expected_home_statuses, "status id: ");
				verify_file(notifications_file, expected_notifications, "notification id: ");
			}

			THEN("The correct last IDs are saved back to the account.")
			{
				std::array<char, 10> id_char_buf;

				REQUIRE(account.second.get_option(user_option::last_home_id) == sv_to_chars(lowest_post_id + mock_get.total_post_count, id_char_buf));
				REQUIRE(account.second.get_option(user_option::last_notification_id) == sv_to_chars(lowest_notif_id + mock_get.total_notif_count, id_char_buf));
			}
		}
	}

	options().clear_accounts();
}