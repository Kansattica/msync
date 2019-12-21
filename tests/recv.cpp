#include <catch2/catch.hpp>

#include "../lib/sync/recv.hpp"
#include "../lib/options/global_options.hpp"

#include "test_helpers.hpp"
#include "mock_network.hpp"
#include "../lib/net_interface/net_interface.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <charconv>
#include <system_error>
#include <tuple>

template <typename make_object>
std::string make_json_array(make_object func, unsigned int min_id, unsigned int max_id)
{
	std::array<char, 10> char_buf;

	std::string toreturn(1, '[');

	// basically, it shouldn't return max_id or min_id itself
	// and the newest (highest ID) goes first
	for (unsigned int id = max_id - 1; id > min_id; id--)
	{
		// see https://en.cppreference.com/w/cpp/utility/to_chars
		// this avoids an allocation compared to std::to_string
		const auto [end, err] = std::to_chars(char_buf.data(), char_buf.data() + char_buf.size(), id);
		if (err != std::errc()) { FAIL("You messed up with to_chars, ya dingus."); }
		toreturn.append(func(std::string_view(char_buf.data(), end - char_buf.data())));
		toreturn.append(1, ',');
	}
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

	unsigned int total_post_count = 210;
	unsigned int total_notif_count = 110;

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

		if (!params.max_id.empty())
		{
			std::from_chars(params.since_id.data(), params.since_id.data() + params.since_id.size(), upper_bound);
		}

		toreturn.message = make_json_array(json_func, lower_bound, upper_bound);

		return toreturn;
	}
};

SCENARIO("Recv downloads and writes the correct number of posts.")
{
	constexpr std::string_view account_name = "user@crime.egg";
	const test_file account_dir = account_directory();
	const auto user_dir = account_dir.filename / account_name;

	auto& account = options().add_new_account(std::string{ account_name });

	REQUIRE(account.first == account_name);

	account.second.set_option(user_option::account_name, "user");
	account.second.set_option(user_option::instance_url, "crime.egg");
	
	GIVEN("A user account with no previously stored information.")
	{

	}
}