#include <catch2/catch.hpp>

#include "test_helpers.hpp"
#include <filesystem.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <locale>

#include <iostream>
#include <iomanip>

#include "../lib/queue/queues.hpp"
#include "../lib/constants/constants.hpp"
#include "../lib/printlog/print_logger.hpp"
#include "../postfile/outgoing_post.hpp"

using namespace std::string_view_literals;

bool prefix_match(std::string_view actual, std::string_view prefix, std::string_view expected)
{
	return std::equal(actual.begin(), actual.begin() + prefix.length(), prefix.begin(), prefix.end()) &&
		std::equal(actual.begin() + prefix.length(), actual.end(), expected.begin(), expected.end());
}

SCENARIO("Queues correctly enqueue and dequeue boosts and favs.")
{
	logs_off = true;
	const test_dir allaccounts = temporary_directory();
	const fs::path account = allaccounts.dirname / "regularguy@internet.egg";
	fs::create_directory(account);
	GIVEN("An empty queue")
	{
		const fs::path queue_file = account / Queue_Filename;

		WHEN("some items are enqueued")
		{
			const auto totest = GENERATE(
				std::make_tuple(queues::boost, "BOOST "sv, "UNBOOST "sv),
				std::make_tuple(queues::fav, "FAV "sv, "UNFAV "sv)
				);

			std::vector<std::string> someids{ "12345", "67890", "123123123123123123123", "longtextboy", "friend" };

			enqueue(std::get<0>(totest), account, std::vector<std::string> { someids });

			THEN("the items are written immediately.")
			{
				const auto lines = print(account);
				REQUIRE(lines.size() == 5);
				REQUIRE(lines == make_expected_ids(someids, std::get<1>(totest)));
			}

			THEN("the file exists.")
			{
				REQUIRE(fs::exists(queue_file));
			}

			AND_WHEN("some of those are dequeued")
			{
				dequeue(std::get<0>(totest), account, std::vector<std::string>{ "12345", "longtextboy" });

				THEN("they're removed from the file.")
				{
					const auto lines = print(account);
					REQUIRE(lines.size() == 3);
					REQUIRE(prefix_match(lines[0], std::get<1>(totest), "67890"));
					REQUIRE(prefix_match(lines[1], std::get<1>(totest), "123123123123123123123"));
					REQUIRE(prefix_match(lines[2], std::get<1>(totest), "friend"));
				}
			}

			AND_WHEN("some of those are dequeued and some aren't in the queue")
			{
				dequeue(std::get<0>(totest), account, std::vector<std::string>{ "12345", "longtextboy", "not in the queue", "other" });

				THEN("the ones in the queue are removed from the file, the ones not in the queue are appended.")
				{
					const auto lines = print(account);
					REQUIRE(lines.size() == 5);
					REQUIRE(prefix_match(lines[0], std::get<1>(totest), "67890"));
					REQUIRE(prefix_match(lines[1], std::get<1>(totest), "123123123123123123123"));
					REQUIRE(prefix_match(lines[2], std::get<1>(totest), "friend"));
					REQUIRE(prefix_match(lines[3], std::get<2>(totest), "not in the queue"));
					REQUIRE(prefix_match(lines[4], std::get<2>(totest), "other"));
				}
			}

			AND_WHEN("some ids that were already in the queue are queued again")
			{
				enqueue(std::get<0>(totest), account, std::vector<std::string> {"12345", "friend", "longtextboy"});

				THEN("the duplicates aren't added to the queue and order is preserved.")
				{
					const auto lines = print(account);
					REQUIRE(lines.size() == 5);
					REQUIRE(lines == make_expected_ids(someids, std::get<1>(totest)));
				}
			}

			AND_WHEN("a mix of ids that were already in the queue and new ones are queued")
			{
				enqueue(std::get<0>(totest), account, std::vector<std::string> {"12345", "a new friend", "longtextboy", "a new friend"});

				THEN("the duplicates aren't added to the queue and order is preserved.")
				{
					const auto lines = print(account);
					REQUIRE(lines.size() == 6);
					REQUIRE(prefix_match(lines[0], std::get<1>(totest), "12345"));
					REQUIRE(prefix_match(lines[1], std::get<1>(totest), "67890"));
					REQUIRE(prefix_match(lines[2], std::get<1>(totest), "123123123123123123123"));
					REQUIRE(prefix_match(lines[3], std::get<1>(totest), "longtextboy"));
					REQUIRE(prefix_match(lines[4], std::get<1>(totest), "friend"));
					REQUIRE(prefix_match(lines[5], std::get<1>(totest), "a new friend"));
				}
			}

			AND_WHEN("the queue is cleared")
			{
				clear(std::get<0>(totest), account);

				THEN("The file is empty, but exists.")
				{
					const auto lines = print(account);
					REQUIRE(fs::exists(queue_file));
					REQUIRE(lines.size() == 0);
				}
			}
		}

	}

}

void files_match(const fs::path& account_dir, const fs::path& original, const std::string& outfile)
{
	const readonly_outgoing_post orig{ original };
	const readonly_outgoing_post newfile{ account_dir / File_Queue_Directory / outfile };

	REQUIRE(orig.parsed.text == newfile.parsed.text);
}


SCENARIO("Queues correctly enqueue and dequeue posts.")
{
	logs_off = true; //shut up the printlogger

	const test_dir allaccounts = temporary_directory();
	const fs::path accountdir = allaccounts.dirname / "queueboy@website.egg";

	const fs::path file_queue_dir = accountdir / File_Queue_Directory;
	const fs::path post_queue_file = accountdir/ Queue_Filename;

	GIVEN("Some posts to enqueue")
	{
		const test_file postfiles[]{ "postboy", "guy.extension", "../up.here", "yeeeeeeehaw" };
		for (auto& file : postfiles)
		{
			std::ofstream of{ file };
			of << "My name is " << file.filename.filename() << "\n";
		}

		WHEN("a post is enqueued")
		{
			const auto idx = GENERATE(0, 1, 2, 3);
			std::vector<std::string> toq{ postfiles[idx].filename.string() };
			std::string justfilename = postfiles[idx].filename.filename().string();

			enqueue(queues::post, accountdir, std::vector<std::string>{toq});

			THEN("the post is copied to the user's account folder")
			{
				files_match(accountdir, postfiles[idx].filename, justfilename);
			}

			THEN("the queue post file is filled correctly")
			{
				const auto lines = print(accountdir);
				REQUIRE(lines.size() == 1);
				REQUIRE(prefix_match(lines[0], "POST ", justfilename));
			}

			AND_WHEN("that post is dequeued")
			{
				dequeue(queues::post, accountdir, std::move(toq));

				CAPTURE(justfilename);
				THEN("msync's copy of the post is deleted")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir / justfilename));
				}

				THEN("the original copy of the post is fine")
				{
					REQUIRE(fs::exists(postfiles[idx].filename));
					const auto lines = read_lines(postfiles[idx].filename);
					REQUIRE(lines.size() == 1);

					std::string compareto{ "My name is \"" };
					compareto.append(justfilename);
					compareto.push_back('\"');
					REQUIRE(lines[0] == compareto);
				}

				THEN("the queue post file is emptied.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 0);
				}
			}

			AND_WHEN("the list is cleared")
			{
				clear(queues::post, accountdir);

				THEN("the queue file is empty.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 0);
				}

				THEN("the queue directory has been erased.")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir));
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

		const std::string expected_text = GENERATE(as<std::string>{}, "", "Hey, check this out");

		{
			outgoing_post op{ files[0].filename };
			op.parsed.text = expected_text;
			op.parsed.attachments = { "attachment.mp3", "filey.png" };
		}

		WHEN("the post is enqueued")
		{
			enqueue(queues::post, accountdir, { "somepost" });

			THEN("the text is as expected")
			{
				files_match(accountdir, files[0].filename, "somepost");
			}

			THEN("the attachments are absolute paths")
			{
				outgoing_post post{ file_queue_dir / "somepost" };
				REQUIRE(post.parsed.attachments.size() == 2);
				REQUIRE(fs::path{ post.parsed.attachments[0] }.is_absolute());
				REQUIRE(fs::path{ post.parsed.attachments[1] }.is_absolute());
			}
		}

	}

	GIVEN("Two different posts with the same name to enqueue")
	{
		const test_file testdir{ "somedir" };
		const test_file postfiles[]{ "thisisapost.hi", "somedir/thisisapost.hi" };
		fs::create_directory(testdir.filename);

		int postno = 1;
		for (const auto& fi : postfiles)
		{
			std::ofstream of{ fi };
			of << "I'm number " << postno++ << '\n';
		}

		WHEN("both are enqueued")
		{
			enqueue(queues::post, accountdir, std::vector<std::string>{ postfiles[0].filename.string(), postfiles[1].filename.string() });

			const fs::path unsuffixedname = file_queue_dir / "thisisapost.hi";
			const fs::path suffixedname = file_queue_dir / "thisisapost.hi.1";

			THEN("one file goes in with the original name, one goes in with a new suffix.")
			{
				REQUIRE(fs::exists(unsuffixedname));
				REQUIRE(fs::exists(suffixedname));
			}

			THEN("the file contents are correct.")
			{
				const auto unsuflines = outgoing_post(unsuffixedname);
				REQUIRE(unsuflines.parsed.text == "I'm number 1");

				const auto suflines = outgoing_post(suffixedname);
				REQUIRE(suflines.parsed.text == "I'm number 2");
			}

			THEN("the queue file is correct.")
			{
				const auto lines = read_lines(post_queue_file);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "POST thisisapost.hi");
				REQUIRE(lines[1] == "POST thisisapost.hi.1");
			}

			THEN("print returns the correct output.")
			{
				const auto lines = print(accountdir);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "POST thisisapost.hi");
				REQUIRE(lines[1] == "POST thisisapost.hi.1");
			}

			AND_WHEN("one is removed")
			{
				const auto idx = GENERATE(0, 1);

				std::string thisfile = idx == 0 ? "thisisapost.hi" : "thisisapost.hi.1";
				std::string otherfile = idx == 1 ? "thisisapost.hi" : "thisisapost.hi.1";

				dequeue(queues::post, accountdir, std::vector<std::string> { thisfile });

				THEN("msync's copy of the dequeued file is deleted.")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir / thisfile));
				}

				THEN("msync's copy of the other file is still there.")
				{
					REQUIRE(fs::exists(file_queue_dir / otherfile));
				}

				THEN("the queue file is updated correctly.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 1);
					REQUIRE(lines[0] == otherfile.insert(0, "POST "));
				}

				THEN("both original files are still there")
				{
					REQUIRE(fs::exists(postfiles[0].filename));
					REQUIRE(fs::exists(postfiles[1].filename));
				}
			}

			AND_WHEN("the list is cleared")
			{
				clear(queues::post, accountdir);

				THEN("the queue file is empty.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 0);
				}

				THEN("the queue directory has been erased.")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir));
				}
			}
		}
	}

	GIVEN("Two different posts with the same base name, but one ends in .bak")
	{
		const test_file postfiles[]{ "thisisapost", "thisisapost.bak" };

		int postno = 1;
		for (const auto& fi : postfiles)
		{
			std::ofstream of{ fi };
			of << "I'm number " << postno++ << '\n';
		}

		WHEN("both are enqueued")
		{
			enqueue(queues::post, accountdir, std::vector<std::string>{ postfiles[0].filename.string(), postfiles[1].filename.string() });

			const fs::path unsuffixedname = file_queue_dir / "thisisapost";
			const fs::path suffixedname = file_queue_dir / "thisisapost.1";

			THEN("one file goes in with the original name, one goes in with a new suffix.")
			{
				REQUIRE(fs::exists(unsuffixedname));
				REQUIRE(fs::exists(suffixedname));
			}

			THEN("the file contents are correct.")
			{
				const auto unsuflines = readonly_outgoing_post(unsuffixedname);
				REQUIRE(unsuflines.parsed.text == "I'm number 1");

				const auto suflines = readonly_outgoing_post(suffixedname);
				REQUIRE(suflines.parsed.text == "I'm number 2");
			}

			THEN("the queue file is correct.")
			{
				const auto lines = read_lines(post_queue_file);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "POST thisisapost");
				REQUIRE(lines[1] == "POST thisisapost.1");
			}

			THEN("print returns the correct output.")
			{
				const auto lines = print(accountdir);
				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "POST thisisapost");
				REQUIRE(lines[1] == "POST thisisapost.1");
			}

			AND_WHEN("one is removed")
			{
				const auto idx = GENERATE(0, 1);

				std::string thisfile = idx == 0 ? "thisisapost" : "thisisapost.1";
				std::string otherfile = idx == 1 ? "thisisapost" : "thisisapost.1";

				dequeue(queues::post, accountdir, std::vector<std::string> { thisfile });

				THEN("msync's copy of the dequeued file is deleted.")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir / thisfile));
				}

				THEN("msync's copy of the other file is still there.")
				{
					REQUIRE(fs::exists(file_queue_dir / otherfile));
				}

				THEN("the queue file is updated correctly.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 1);
					REQUIRE(lines[0] == otherfile.insert(0, "POST "));
				}

				THEN("both original files are still there")
				{
					REQUIRE(fs::exists(postfiles[0].filename));
					REQUIRE(fs::exists(postfiles[1].filename));
				}
			}

			AND_WHEN("the list is cleared")
			{
				clear(queues::post, accountdir);

				THEN("the queue file is empty.")
				{
					const auto lines = read_lines(post_queue_file);
					REQUIRE(lines.size() == 0);
				}

				THEN("the queue directory has been erased.")
				{
					REQUIRE_FALSE(fs::exists(file_queue_dir));
				}
			}
		}
	}
}

SCENARIO("Queues can handle a mix of different queued calls.")
{
	const test_dir allaccounts = temporary_directory();
	const fs::path accountdir = allaccounts.dirname / "funnybone@typical.egg";
	fs::create_directory(accountdir);

	const fs::path file_queue_dir = accountdir / File_Queue_Directory;
	const fs::path queue_file = accountdir / Queue_Filename;

	GIVEN("An empty queue and some some posts to enqueue.")
	{
		const test_file to_queue[] { "one post", "another.post!" };

		int postno = 1;
		for (const auto& fi : to_queue)
		{
			std::ofstream of{ fi };
			of << "hey, I'm number " << postno++ << '\n';
		}

		WHEN("A mix of favs, boosts, and posts are added and removed, the file reflects that.")
		{
			REQUIRE(!fs::exists(queue_file));

			dequeue(queues::post, accountdir, { "69420", "somepost", "a real lousy one" });

			REQUIRE(read_lines(queue_file) == std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one"});

			enqueue(queues::boost, accountdir, { "boosty", "cool guy", "friend!", "someone else" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST cool guy", "BOOST friend!", "BOOST someone else" });

			enqueue(queues::fav, accountdir, { "favvy", "cool guy", "friend!" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST cool guy", "BOOST friend!", "BOOST someone else",
					"FAV favvy", "FAV cool guy", "FAV friend!" });

			dequeue(queues::boost, accountdir, { "someone else", "cool guy", "whoopsie" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST friend!",
					"FAV favvy", "FAV cool guy", "FAV friend!",
					"UNBOOST whoopsie"});

			enqueue(queues::post, accountdir, { "one post" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST friend!",
					"FAV favvy", "FAV cool guy", "FAV friend!",
					"UNBOOST whoopsie",
					"POST one post"});

			REQUIRE(read_lines(file_queue_dir / "one post") == std::vector<std::string> { "visibility=default", "--- post body below this line ---", "hey, I'm number 1" });

			dequeue(queues::fav, accountdir, { "friend!", "whoopsie", "sorry about that" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST friend!",
					"FAV favvy", "FAV cool guy",
					"UNBOOST whoopsie",
					"POST one post",
					"UNFAV whoopsie", "UNFAV sorry about that"});

			enqueue(queues::fav, accountdir, { "sorry about that" });

			REQUIRE(read_lines(queue_file) == 
				std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
					"BOOST boosty", "BOOST friend!",
					"FAV favvy", "FAV cool guy",
					"UNBOOST whoopsie",
					"POST one post",
					"UNFAV whoopsie", "UNFAV sorry about that",
					"FAV sorry about that"});

			AND_WHEN("The post queue is cleared.")
			{
				clear(queues::post, accountdir);

				THEN("The file is as expected.")
				{
					REQUIRE(read_lines(queue_file) == std::vector<std::string>{
						"BOOST boosty", "BOOST friend!",
						"FAV favvy", "FAV cool guy",
						"UNBOOST whoopsie",
						"UNFAV whoopsie", "UNFAV sorry about that",
						"FAV sorry about that"});
				}

				THEN("The corresponding post directory was deleted, too.")
				{
					REQUIRE(!fs::exists(file_queue_dir));
				}
			}

			AND_WHEN("The fav queue is cleared.")
			{
				clear(queues::fav, accountdir);

				THEN("The file is as expected.")
				{
					REQUIRE(read_lines(queue_file) ==
						std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
						"BOOST boosty", "BOOST friend!",
						"UNBOOST whoopsie",
						"POST one post"});
				}
			}

			AND_WHEN("The boost queue is cleared.")
			{
				clear(queues::boost, accountdir);

				THEN("The file is as expected.")
				{
					REQUIRE(read_lines(queue_file) ==
						std::vector<std::string>{"UNPOST 69420", "UNPOST somepost", "UNPOST a real lousy one",
						"FAV favvy", "FAV cool guy",
						"POST one post",
						"UNFAV whoopsie", "UNFAV sorry about that",
						"FAV sorry about that"});
				}
			}
		}
	}
}

SCENARIO("Can enqueue and dequeue files with non-ASCII paths.")
{
	std::locale::global(std::locale("en_US.UTF-8"));

	GIVEN("Some files with non-ASCII paths.")
	{
		const test_dir skunkzone = u8"cool🦨zone";
		for (const auto c : skunkzone.dirname.native())
		{
			std::cout << std::hex << (int)c << ' ';
		}
		std::cout << '\n';

		for (const auto filename : { u8"a friend.txt", u8"your 🤠 friend.txt" })
		{
			std::ofstream fi{ filename };
			const fs::path infolder = skunkzone.dirname / filename;
			std::ofstream folderfi{ infolder.c_str() }; // Boost insists.

			fi << "Hi, I'm " << filename;
			folderfi << "Hi, I'm " << filename << u8" in a 😎 cool folder.";
		}

		WHEN("The files are enqueued.")
		{
			const auto allaccounts = temporary_directory();
			const fs::path accountdir = allaccounts.dirname / "anonymous@crime.egg";
			const fs::path file_queue_dir = accountdir / File_Queue_Directory;
			const fs::path queue_file = accountdir / Queue_Filename;

			enqueue(queues::post, accountdir, std::vector<std::string> {
				u8"a friend.txt", u8"your 🤠 friend.txt",
				u8"cool🦨zone/a friend.txt", u8"cool🦨zone/your 🤠 friend.txt"
			});

			THEN("The queue file has the correct filenames in the correct order.")
			{
				REQUIRE(read_lines(queue_file) == std::vector<std::string> {
					u8"POST a friend.txt",
					u8"POST your 🤠 friend.txt",
					u8"POST a friend.txt.1",
					u8"POST your 🤠 friend.txt.1",
				});
			}

			THEN("The copied files have their contents correct.")
			{
				REQUIRE(readonly_outgoing_post(file_queue_dir / u8"a friend.txt").parsed.text == "Hi, I'm a friend.txt");
				REQUIRE(readonly_outgoing_post(file_queue_dir / u8"your 🤠 friend.txt").parsed.text == "Hi, I'm your 🤠 friend.txt");
				REQUIRE(readonly_outgoing_post(file_queue_dir / u8"a friend.txt.1").parsed.text == "Hi, I'm a friend.txt in a 😎 cool folder.");
				REQUIRE(readonly_outgoing_post(file_queue_dir / u8"your 🤠 friend.txt.1").parsed.text == "Hi, I'm your 🤠 friend.txt in a 😎 cool folder.");
			}

			AND_WHEN("Some of those files are dequeued.")
			{
				dequeue(queues::post, accountdir, std::vector<std::string> {
						 u8"your 🤠 friend.txt", u8"a friend.txt.1"
				});

				THEN("The queue file has the correct filenames in the correct order.")
				{
					REQUIRE(read_lines(queue_file) == std::vector<std::string> {
						u8"POST a friend.txt", u8"POST your 🤠 friend.txt.1"
					});
				}

				THEN("The remaining files have their contents correct.")
				{
					REQUIRE(readonly_outgoing_post(file_queue_dir / u8"a friend.txt").parsed.text == "Hi, I'm a friend.txt");
					REQUIRE_FALSE(fs::exists(file_queue_dir / u8"your 🤠 friend.txt"));
					REQUIRE_FALSE(fs::exists(file_queue_dir / u8"a friend.txt.1"));
					REQUIRE(readonly_outgoing_post(file_queue_dir / u8"your 🤠 friend.txt.1").parsed.text == "Hi, I'm your 🤠 friend.txt in a 😎 cool folder.");
				}
			}

		}
	}

}
