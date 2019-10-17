#include <catch2/catch.hpp>

#include "test_helpers.hpp"
#include <filesystem.hpp>
#include <fstream>
#include <vector>
#include <string>

#include "../lib/queue/queues.hpp"
#include "../lib/constants/constants.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/printlog/print_logger.hpp"
#include "../postfile/outgoing_post.hpp"

SCENARIO("Queues correctly enqueue and dequeue boosts and favs.")
{
	const std::string account = "regularguy@internet.egg";
	GIVEN("An empty queue")
	{
		test_file allaccounts = options().executable_location / Account_Directory; //make sure this gets cleaned up, too
		test_file accountdir = options().executable_location / Account_Directory / account;

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
				auto lines = print(totest.first, account);
				REQUIRE(lines.size() == 5);
				REQUIRE(lines == someids);
			}

			THEN("the file exists.")
			{
				REQUIRE(fs::exists(accountdir.filename / totest.second));
			}

			AND_WHEN("some of those are dequeued")
			{
				std::vector<std::string> removethese{ "12345", "longtextboy" };
				dequeue(totest.first, account, std::move(removethese));

				THEN("they're removed from the file.")
				{
					auto lines = print(totest.first, account);
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
					auto lines = print(totest.first, account);
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
					auto lines = print(totest.first, account);
					REQUIRE(fs::exists(accountdir.filename / totest.second));
					REQUIRE(lines.size() == 0);
				}
			}
		}

	}

}

void files_match(const std::string& account, const fs::path& original, const std::string& outfile)
{
	outgoing_post orig{ original };
	outgoing_post newfile{ options().executable_location / Account_Directory / account / File_Queue_Directory / outfile };

	REQUIRE(orig.parsed.text == newfile.parsed.text);
}


SCENARIO("Queues correctly enqueue and dequeue posts.")
{
	logs_off = true; //shut up the printlogger

	const std::string account = "queueboy@website.egg";
	test_file accountdir = options().executable_location / Account_Directory / account;
	test_file allaccounts = options().executable_location / Account_Directory;

	GIVEN("Some posts to enqueue")
	{
		const test_file postfiles[]{ test_file{"postboy"}, test_file{"guy.extension"}, test_file{"../up.here"}, test_file{"yeeeeeeehaw"} };
		for (auto& file : postfiles)
		{
			std::ofstream of{ file.filename };
			of << "My name is " << file.filename.filename() << "\n";
		}

		WHEN("a post is enqueued")
		{
			auto idx = GENERATE(0, 1, 2, 3);
			std::vector<std::string> toq{ postfiles[idx].filename.string() };
			std::string justfilename = postfiles[idx].filename.filename().string();

			enqueue(queues::post, account, toq);

			THEN("the post is copied to the user's account folder")
			{
				files_match(account, postfiles[idx].filename, justfilename);
			}

			THEN("the queue post file is filled correctly")
			{
				auto lines = print(queues::post, account);
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

			AND_WHEN("the list is cleared")
			{
				clear(queues::post, account);

				THEN("the queue file is empty.")
				{
					auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
					REQUIRE(lines.size() == 0);
				}

				THEN("the queue directory has been erased.")
				{
					REQUIRE_FALSE(fs::exists(accountdir.filename / File_Queue_Directory));
				}
			}
		}
	}

	GIVEN("A post with attachments to enqueue")
	{
		const test_file files[]{ "somepost", "attachment.mp3", "filey.png" };

		// make these files exist
		for (int i = 1; i < 3; i++)
		{
			std::ofstream of{ files[i] };
			of << "I'm file number " << i;
		}

		std::string expected_text = GENERATE(as<std::string>{}, "", "Hey, check this out");

		{
			outgoing_post op{ files[0].filename };
			op.parsed.text = expected_text;
			op.parsed.attachments = { "attachment.mp3", "filey.png" };
		}

		WHEN("the post is enqueued")
		{
			enqueue(queues::post, account, { "somepost" });

			THEN("the text is as expected")
			{
				files_match(account, files[0].filename, "somepost");
			}

			THEN("the attachments are absolute paths")
			{
				outgoing_post post{ accountdir.filename / File_Queue_Directory / "somepost" };
				REQUIRE(post.parsed.attachments.size() == 2);
				REQUIRE(fs::path{ post.parsed.attachments[0] }.is_absolute());
				REQUIRE(fs::path{ post.parsed.attachments[1] }.is_absolute());
			}
		}

	}

	GIVEN("Two different posts with the same name to enqueue")
	{
		const test_file testdir{ "somedir" };
		const test_file postfiles[]{ test_file{"thisisapost.hi"}, test_file{"somedir/thisisapost.hi"} };
		fs::create_directory(testdir.filename);

		int postno = 1;
		for (auto& fi : postfiles)
		{
			std::ofstream of{ fi.filename };
			of << "I'm number " << postno++ << '\n';
		}

		WHEN("both are enqueued")
		{
			enqueue(queues::post, account, std::vector<std::string>{ postfiles, postfiles + 2 });

			const fs::path unsuffixedname = accountdir.filename / File_Queue_Directory / "thisisapost.hi";
			const fs::path suffixedname = accountdir.filename / File_Queue_Directory / "thisisapost.hi.1";

			THEN("one file goes in with the original name, one goes in with a new suffix.")
			{
				REQUIRE(fs::exists(unsuffixedname));
				REQUIRE(fs::exists(suffixedname));
			}

			THEN("the file contents are correct.")
			{
				auto unsuflines = outgoing_post(unsuffixedname);
				REQUIRE(unsuflines.parsed.text == "I'm number 1");

				auto suflines = outgoing_post(suffixedname);
				REQUIRE(suflines.parsed.text == "I'm number 2");
			}

			THEN("the queue file is correct.")
			{
				auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "thisisapost.hi");
				REQUIRE(lines[1] == "thisisapost.hi.1");
			}

			THEN("print returns the correct output.")
			{
				auto lines = print(queues::post, account);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "thisisapost.hi");
				REQUIRE(lines[1] == "thisisapost.hi.1");
			}

			AND_WHEN("one is removed")
			{
				auto idx = GENERATE(0, 1);

				std::string thisfile = idx == 0 ? "thisisapost.hi" : "thisisapost.hi.1";
				std::string otherfile = idx == 1 ? "thisisapost.hi" : "thisisapost.hi.1";

				dequeue(queues::post, account, std::vector<std::string> { thisfile });

				THEN("msync's copy of the dequeued file is deleted.")
				{
					REQUIRE_FALSE(fs::exists(accountdir.filename / File_Queue_Directory / thisfile));
				}

				THEN("msync's copy of the other file is still there.")
				{
					REQUIRE(fs::exists(accountdir.filename / File_Queue_Directory / otherfile));
				}

				THEN("the queue file is updated correctly.")
				{
					auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
					REQUIRE(lines.size() == 1);
					REQUIRE(lines[0] == otherfile);
				}

				THEN("both original files are still there")
				{
					REQUIRE(fs::exists(postfiles[0].filename));
					REQUIRE(fs::exists(postfiles[1].filename));
				}
			}

			AND_WHEN("the list is cleared")
			{
				clear(queues::post, account);

				THEN("the queue file is empty.")
				{
					auto lines = read_lines(accountdir.filename / Post_Queue_Filename);
					REQUIRE(lines.size() == 0);
				}

				THEN("the queue directory has been erased.")
				{
					REQUIRE_FALSE(fs::exists(accountdir.filename / File_Queue_Directory));
				}
			}
		}
	}
}
