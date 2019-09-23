#include <catch2/catch.hpp>

#include "test_helpers.hpp"
#include <filesystem.hpp>
#include <vector>
#include <string>

#include "../lib/queue/queues.hpp"
#include "../lib/constants/constants.hpp"

SCENARIO("Queues correctly enqueue and dequeue boosts and favs.")
{
	const std::string account = "regularguy@internet.egg";
	GIVEN("An empty queue")
	{
		test_file accountdir = fs::current_path() / account;
		fs::create_directory(accountdir.filename);

		WHEN("some items are enqueued")
		{
			auto totest = GENERATE(
				std::make_pair(queues::boost, Boost_Queue_Filename),
				std::make_pair(queues::fav, Fav_Queue_Filename)
			);

			std::vector<std::string> someids{ "12345", "67890", "123123123123123123123", "longtextboy", "friend" };
			enqueue(totest.first, account, someids);

			THEN("the items are written immediately.")
			{
				auto lines = read_lines(accountdir.filename / totest.second);
				REQUIRE(lines.size() == 5);
				REQUIRE(lines == someids);
			}

			AND_WHEN("some of those are dequeued")
			{
				std::vector<std::string> removethese{ "12345", "longtextboy" };
				dequeue(totest.first, account, std::move(removethese));

				THEN("they're removed from the file.")
				{
					auto lines = read_lines(accountdir.filename / totest.second);
					REQUIRE(lines.size() == 3);
					REQUIRE(lines[0] == "67890");
					REQUIRE(lines[1] == "123123123123123123123");
					REQUIRE(lines[2] == "friend");
				}
			}

			AND_WHEN("some of those are dequeued and some aren't in the queue")
			{
				std::vector<std::string> removethese{ "12345", "longtextboy", "not in the queue", "other" };
				dequeue(totest.first, account, std::move(removethese));

				THEN("the ones in the queue are removed from the file, the ones not in the queue are appended.")
				{
					auto lines = read_lines(accountdir.filename / totest.second);
					REQUIRE(lines.size() == 5);
					REQUIRE(lines[0] == "67890");
					REQUIRE(lines[1] == "123123123123123123123");
					REQUIRE(lines[2] == "friend");
					REQUIRE(lines[3] == "not in the queue-");
					REQUIRE(lines[4] == "other-");
				}
			}

			AND_WHEN("the queue is cleared")
			{
				clear(totest.first, account);

				THEN("The file is empty, but exists.")
				{
					auto lines = read_lines(accountdir.filename / totest.second);
					REQUIRE(lines.size() == 0);
				}
			}
		}

	}

}
