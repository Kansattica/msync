#include <catch2/catch.hpp>

#include "test_helpers.hpp"
#include <filesystem.hpp>
#include <vector>
#include <string>

#include "../lib/queue/queues.hpp"
#include "../lib/constants/constants.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/printlog/print_logger.hpp"

SCENARIO("Queues correctly enqueue and dequeue boosts and favs.")
{
	const std::string account = "regularguy@internet.egg";
	GIVEN("An empty queue")
	{
		test_file accountdir = options.executable_location / account;
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

void files_match(const std::string& account, const std::string& filename)
{
	fs::path relative = fs::path{account} / filename;
	REQUIRE(read_lines(fs::current_path() / relative) == read_lines(options.executable_location / relative));
}

SCENARIO("Queues correctly enqueue and dequeue posts.")
{
	logs_off = true; //shut up the printlogger

	const std::string account = "queueboy@website.egg";
	test_file accountdir = options.executable_location / account;
	fs::create_directory(accountdir.filename);
	GIVEN("Some posts to enqueue")
	{
		const test_file postfiles[]{ "postboy", "guy.extension", "../up.here", "yeeeeeeehaw" };
		for (auto& file : postfiles)
		{
			std::ofstream of{ file.filename };
			of << "My name is " << file.filename.filename() << "\n";
		}

		WHEN("a post is enqueued")
		{
			auto idx = GENERATE(0, 1, 2, 3);
			std::vector<std::string> toq { postfiles[idx].filename.string() };
			std::string justfilename = postfiles[idx].filename.filename().string();

			enqueue(queues::post, account, toq);

			THEN("the post is copied to the user's account folder")
			{
				files_match(account, justfilename);
			}

			THEN("the queue post file is filled correctly")
			{
				auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
				REQUIRE(lines.size() == 1);
				REQUIRE(lines[0] == justfilename);
			}

			AND_WHEN("that post is dequeued")
			{
				dequeue(queues::post, account, std::move(toq));

				THEN("msync's copy of the post is deleted")
				{
					REQUIRE_FALSE(fs::exists(accountdir.filename / File_Queue_Directory / justfilename));
				}

				THEN("the original copy of the post is fine")
				{
					REQUIRE(fs::exists(postfiles[idx].filename));
					auto lines = read_lines(postfiles[idx].filename);
					REQUIRE(lines.size() == 1);

					std::string compareto{ "My name is \"" };
					compareto.append(justfilename);
					compareto.push_back('\"');
					REQUIRE(lines[0] == compareto);
				}

				THEN("the queue post file is emptied.")
				{
					auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
					REQUIRE(lines.size() == 0);
				}
			}
		}
	}
}
