#include <catch2/catch.hpp>

#include "../lib/sync/send.hpp"
#include "../lib/net_interface/net_interface.hpp"
#include "../lib/queue/queues.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/constants/constants.hpp"

#include "test_helpers.hpp"

#include <string_view>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <print_logger.hpp>

struct mock_args
{
	std::string url;
	std::string access_token;
	status_params params;
};

struct mock_network
{
	int status_code = 200;
	bool fatal_error = false;

	void set_succeed_after(size_t n)
	{
		succeed_after = succeed_after_n = n;
	}

	std::vector<mock_args> arguments;
	
	net_response mock_post(std::string_view url, std::string_view access_token)
	{
		arguments.push_back(mock_args{ std::string {url}, std::string { access_token } });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		return toreturn;
	}

	net_response mock_delete(std::string_view url, std::string_view access_token)
	{
		arguments.push_back(mock_args{ std::string {url}, std::string { access_token } });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		return toreturn;
	}

	net_response mock_new_status(std::string_view url, std::string_view access_token, status_params params)
	{
		arguments.push_back(mock_args{ std::string {url}, std::string { access_token }, std::move(params) });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		return toreturn;
	}

private:
	size_t succeed_after_n = 1;
	size_t succeed_after = succeed_after_n;
};

struct mock_network_post : public mock_network
{
	net_response operator()(std::string_view url, std::string_view access_token)
	{
		return mock_post(url, access_token);
	}
};

struct mock_network_delete : public mock_network
{
	net_response operator()(std::string_view url, std::string_view access_token)
	{
		return mock_delete(url, access_token);
	}
};

struct mock_network_new_status : public mock_network
{
	net_response operator()(std::string_view url, std::string_view access_token, status_params params)
	{
		return mock_new_status(url, access_token, std::move(params));
	}
};

std::string make_expected_url(const std::string_view id, const std::string_view route, const std::string_view instance_url)
{
	std::string toreturn{ "https://" };
	toreturn.append(instance_url).append("/api/v1/statuses/").append(id).append(route);
	return toreturn;
}

std::vector<std::string_view> repeat_each_element(const std::vector<std::string>& in, size_t count)
{
	std::vector<std::string_view> toreturn;
	for (const auto& str : in)
	{
		for (size_t i = 0; i < count; i++)
		{
			toreturn.emplace_back(str);
		}
	}
	return toreturn;
}


SCENARIO("Send correctly sends from and modifies the queue with favs and boosts.")
{
	logs_off = true;

	const test_file fi = account_directory();

	const auto queue = GENERATE(
		std::make_tuple(queues::fav, "/favourite", "/unfavourite"),
		std::make_tuple(queues::boost, "/reblog", "/unreblog"));

	const std::vector<std::string> testvect = GENERATE(
		std::vector<std::string>{ "someid", "someotherid", "mrid" },
		std::vector<std::string>{},
		std::vector<std::string>{ "justone" });

	constexpr std::string_view account = "someguy@cool.account";
	constexpr std::string_view instanceurl = "cool.account";
	constexpr std::string_view accesstoken = "sometoken";

	GIVEN("A queue with some ids to add and a good connection")
	{
		enqueue(std::get<0>(queue), account, testvect);

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mock_network_delete mockdel;
			mock_network_new_status mocknew;

			auto send = send_posts{ mockpost, mockdel, mocknew };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(std::get<0>(queue), account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::equal(mockpost.arguments.begin(), mockpost.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, std::get<1>(queue), instanceurl);
					}));

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
			}
		}
	}

	GIVEN("A queue with some ids to remove and a good connection")
	{
		std::vector<std::string> toremove{ testvect };
		std::for_each(toremove.begin(), toremove.end(), [](auto& str) { str.push_back('-'); });

		enqueue(std::get<0>(queue), account, toremove);

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mock_network_delete mockdel;
			mock_network_new_status mocknew;

			auto send = send_posts{ mockpost, mockdel, mocknew };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(std::get<0>(queue), account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mockpost.arguments.size() == toremove.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				//testvect doesn't have the trailing minus signs, which is what we want for this test.
				REQUIRE(std::equal(mockpost.arguments.begin(), mockpost.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, std::get<2>(queue), instanceurl);
					}));

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
			}
		}
	}

	GIVEN("A queue with some ids to add and retryable errors that ultimately succeed")
	{
		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		const auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));

		enqueue(std::get<0>(queue), account, testvect);

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mockpost.set_succeed_after(retries.second);

			mock_network_delete mockdel;
			mock_network_new_status mocknew;

			auto send = send_posts{ mockpost, mockdel, mocknew };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(std::get<0>(queue), account).empty());
			}

			THEN("each ID was tried the correct number of times.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size() * retries.second);
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				auto repeated = repeat_each_element(testvect, retries.second);
				REQUIRE(std::equal(mockpost.arguments.begin(), mockpost.arguments.end(), repeated.begin(), repeated.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, std::get<1>(queue), instanceurl);
					}));

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
			}
		}
	}

	GIVEN("A queue where all the IDs fail")
	{
		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		const auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));


		enqueue(std::get<0>(queue), account, testvect);

		WHEN("the queue is sent")
		{

			mock_network_post mockpost;
			mockpost.fatal_error = true;
			mockpost.status_code = 500;

			mock_network_delete mockdel;
			mock_network_new_status mocknew;

			auto send = send_posts{ mockpost, mockdel, mocknew };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue hasn't changed.")
			{
				REQUIRE(print(std::get<0>(queue), account) == testvect);
			}

			THEN("each ID was tried once.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::equal(mockpost.arguments.begin(), mockpost.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, std::get<1>(queue), instanceurl);
					}));

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
			}
		}
	}
}

SCENARIO("Send correctly sends new posts and deletes existing ones.")
{
	GIVEN("A queue with some post filenames to send.")
	{

	}

}
