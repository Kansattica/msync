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
	
	net_response mock_post(const std::string_view url, const std::string_view access_token)
	{
		arguments.push_back(mock_args{ std::string {url}, std::string { access_token } });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		return toreturn;
	}

	net_response operator()(const std::string_view url, const std::string_view access_token)
	{
		return mock_post(url, access_token);
	}

private:
	size_t succeed_after_n = 1;
	size_t succeed_after = succeed_after_n;
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
	GIVEN("A queue with some ids to add and a good connection")
	{
		auto queue = GENERATE(
			std::make_pair(queues::fav, "/favourite"),
			std::make_pair(queues::boost, "/reblog"));
		std::vector<std::string> testvect = GENERATE( 
			std::vector<std::string>{ "someid", "someotherid", "mrid" },
			std::vector<std::string>{},
			std::vector<std::string>{ "justone" });

		constexpr std::string_view account = "someguy@cool.account";
		constexpr std::string_view instanceurl = "cool.account";
		constexpr std::string_view accesstoken = "sometoken";

		enqueue(queue.first, account, testvect);

		WHEN("the queue is sent")
		{
			mock_network mock;

			send_posts<mock_network, void, void> send{ mock };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(queue.first, account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mock.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mock.arguments.begin(), mock.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::equal(mock.arguments.begin(), mock.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, queue.second, instanceurl);
					}));

			}
		}
	}

	GIVEN("A queue with some ids to remove and a good connection")
	{
		auto queue = GENERATE(
			std::make_pair(queues::fav, "/unfavourite"),
			std::make_pair(queues::boost, "/unreblog"));
		std::vector<std::string> testvect = GENERATE( 
			std::vector<std::string>{ "someid-", "someotherid-", "mrid-" },
			std::vector<std::string>{},
			std::vector<std::string>{ "justone-" });

		constexpr std::string_view account = "someguy@cool.account";
		constexpr std::string_view instanceurl = "cool.account";
		constexpr std::string_view accesstoken = "sometoken";

		enqueue(queue.first, account, testvect);

		WHEN("the queue is sent")
		{
			mock_network mock;

			send_posts<mock_network, void, void> send{ mock };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(queue.first, account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mock.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mock.arguments.begin(), mock.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::equal(mock.arguments.begin(), mock.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, auto& expected)
					{
						expected.pop_back(); //remove that minus sign
						return actual.url == make_expected_url(expected, queue.second, instanceurl);
					}));

			}
		}
	}

	GIVEN("A queue with some ids to add and retryable errors that ultimately succeed")
	{
		auto queue = GENERATE(
			std::make_pair(queues::fav, "/favourite"),
			std::make_pair(queues::boost, "/reblog"));
		std::vector<std::string> testvect = GENERATE( 
			std::vector<std::string>{ "someid", "someotherid", "mrid" },
			std::vector<std::string>{},
			std::vector<std::string>{ "justone" });

		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));

		constexpr std::string_view account = "someguy@cool.account";
		constexpr std::string_view instanceurl = "cool.account";
		constexpr std::string_view accesstoken = "sometoken";

		enqueue(queue.first, account, testvect);

		WHEN("the queue is sent")
		{
			mock_network mock;
			mock.set_succeed_after(retries.second);

			send_posts<mock_network, void, void> send{ mock };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(queue.first, account).empty());
			}

			THEN("each ID was tried the correct number of times.")
			{
				REQUIRE(mock.arguments.size() == testvect.size() * retries.second);
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mock.arguments.begin(), mock.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				auto repeated = repeat_each_element(testvect, retries.second);
				REQUIRE(std::equal(mock.arguments.begin(), mock.arguments.end(), repeated.begin(), repeated.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, queue.second, instanceurl);
					}));

			}
		}
	}

	GIVEN("A queue where all the IDs fail")
	{
		auto queue = GENERATE(
			std::make_pair(queues::fav, "/favourite"),
			std::make_pair(queues::boost, "/reblog"));
		std::vector<std::string> testvect = GENERATE( 
			std::vector<std::string>{ "someid", "someotherid", "mrid" },
			std::vector<std::string>{},
			std::vector<std::string>{ "justone" });

		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));

		constexpr std::string_view account = "someguy@cool.account";
		constexpr std::string_view instanceurl = "cool.account";
		constexpr std::string_view accesstoken = "sometoken";

		enqueue(queue.first, account, testvect);

		WHEN("the queue is sent")
		{
			mock_network mock;
			mock.fatal_error = true;
			mock.status_code = 500;

			send_posts<mock_network, void, void> send{ mock };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue hasn't changed.")
			{
				REQUIRE(print(queue.first, account) == testvect);
			}

			THEN("each ID was tried once.")
			{
				REQUIRE(mock.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mock.arguments.begin(), mock.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::equal(mock.arguments.begin(), mock.arguments.end(), testvect.begin(), testvect.end(), [&](const auto& actual, const auto& expected)
					{
						return actual.url == make_expected_url(expected, queue.second, instanceurl);
					}));

			}
		}
	}
}

