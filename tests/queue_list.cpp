#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/queue/queue_list.hpp"

#include <filesystem.hpp>

#include <fstream>
#include <string>
#include <array>
#include <algorithm>

SCENARIO("queue_lists save their data when destroyed.")
{
	GIVEN("An queue_list with some values.")
	{
		const test_file tf = temporary_file();

		queue_list opts(tf.filename);
		opts.parsed.push_back(api_call{ api_route::fav, "thingone" });
		opts.parsed.push_back(api_call{ api_route::unboost, "thingtwo" });

		REQUIRE(opts.parsed.size() == 2);

		WHEN("an queue_list is moved from")
		{
			queue_list newopts(std::move(opts));

			THEN("no file is written")
			{
				REQUIRE_FALSE(fs::exists(tf.filename));
			}
		}

		WHEN("an queue_list is moved from with the move constructor and destroyed")
		{
			{
				queue_list newopts(std::move(opts));
			}

			THEN("the file is written")
			{
				REQUIRE(fs::exists(tf.filename));

				auto lines = read_lines(tf.filename);

				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "FAV thingone");
				REQUIRE(lines[1] == "UNBOOST thingtwo");
			}
		}

		WHEN("an queue_list is moved from with move assignment and destroyed")
		{
			{
				queue_list newopts = std::move(opts);
			}

			THEN("the file gets written")
			{
				REQUIRE(fs::exists(tf.filename));

				auto lines = read_lines(tf.filename);

				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "FAV thingone");
				REQUIRE(lines[1] == "UNBOOST thingtwo");
			}
		}

		WHEN("an item is popped and the queue_list is destroyed")
		{
			opts.parsed.pop_front();

			{
				queue_list newopts = std::move(opts);
			}

			THEN("the file gets written without the deleted options.")
			{
				REQUIRE(fs::exists(tf.filename));

				auto lines = read_lines(tf.filename);

				REQUIRE(lines.size() == 1);
				REQUIRE(lines[0] == "UNBOOST thingtwo");
			}
		}
	}
}

SCENARIO("queue_lists read data when created.")
{
	GIVEN("An queue_list on disk with some data.")
	{
		const test_file tf = temporary_file();

		{
			std::ofstream fout(tf);
			fout << "POST firsthing\n";
			fout << "UNPOST secondthing\n";
			fout << "UNPOST thirdthing\n";
		}

		WHEN("an queue_list is created")
		{
			queue_list testfi(tf.filename);

			THEN("it has the parsed information from the file.")
			{
				REQUIRE(testfi.parsed.size() == 3);
				REQUIRE(testfi.parsed.front() == api_call{ api_route::post, "firsthing" });
				testfi.parsed.pop_front();
				REQUIRE(testfi.parsed.front() == api_call{ api_route::unpost, "secondthing" });
				testfi.parsed.pop_front();
				REQUIRE(testfi.parsed.front() == api_call{ api_route::unpost, "thirdthing" });
			}
		}

		WHEN("an queue_list is opened and modified")
		{
			{
				queue_list testfi(tf.filename);

				testfi.parsed.pop_front();
				testfi.parsed.push_back(api_call{ api_route::fav, ":) :)" });
				testfi.parsed.push_back(api_call{ api_route::unfav, ":) :4" });
				REQUIRE(testfi.parsed.size() == 4);
			}

			THEN("it saves the new information back to the file.")
			{
				auto lines = read_lines(tf.filename);

				REQUIRE(lines.size() == 4);
				REQUIRE(lines[0] == "UNPOST secondthing");
				REQUIRE(lines[1] == "UNPOST thirdthing");
				REQUIRE(lines[2] == "FAV :) :)");
				REQUIRE(lines[3] == "UNFAV :) :4");

				AND_THEN("The original file is backed up.")
				{
					REQUIRE(fs::exists(tf.filenamebak));

					auto linesbak = read_lines(tf.filenamebak);

					REQUIRE(linesbak.size() == 3);
					REQUIRE(linesbak[0] == "POST firsthing");
					REQUIRE(linesbak[1] == "UNPOST secondthing");
					REQUIRE(linesbak[2] == "UNPOST thirdthing");
				}
			}
		}
	}

	GIVEN("An queue_list on disk with some data and some stuff to skip.")
	{
		const test_file tf = temporary_file();

		{
			std::ofstream fout(tf);
			fout << "POST firsthing\n";
			fout << '\n';
			fout << "BOOST secondthing\n";
			fout << "# some comment for you\n";
			fout << "FAV thirdthing\n";
		}

		WHEN("an queue_list is created")
		{
			queue_list testfi(tf.filename);

			THEN("it has the parsed information from the file.")
			{
				REQUIRE(testfi.parsed.size() == 3);
				REQUIRE(testfi.parsed.front() == api_call{ api_route::post, "firsthing" });
				testfi.parsed.pop_front();
				REQUIRE(testfi.parsed.front() == api_call{ api_route::boost, "secondthing" });
				testfi.parsed.pop_front();
				REQUIRE(testfi.parsed.front() == api_call{ api_route::fav, "thirdthing" });
			}
		}

		WHEN("an queue_list is opened and modified")
		{
			{
				queue_list testfi(tf.filename);

				testfi.parsed.pop_front();
				testfi.parsed.push_back(api_call{ api_route::fav, ":) :)" });
				testfi.parsed.push_back(api_call{ api_route::post, ":) :4" });
				REQUIRE(testfi.parsed.size() == 4);
			}

			THEN("it saves the new information back to the file.")
			{
				const auto lines = read_lines(tf.filename);

				REQUIRE(lines.size() == 4);
				REQUIRE(lines[0] == "BOOST secondthing");
				REQUIRE(lines[1] == "FAV thirdthing");
				REQUIRE(lines[2] == "FAV :) :)");
				REQUIRE(lines[3] == "POST :) :4");

				AND_THEN("The original file is backed up.")
				{
					REQUIRE(fs::exists(tf.filenamebak));

					const auto linesbak = read_lines(tf.filenamebak);

					REQUIRE(linesbak.size() == 5);
					REQUIRE(linesbak[0] == "POST firsthing");
					REQUIRE(linesbak[1] == "");
					REQUIRE(linesbak[2] == "BOOST secondthing");
					REQUIRE(linesbak[3] == "# some comment for you");
					REQUIRE(linesbak[4] == "FAV thirdthing");
				}
			}
		}
	}
}

SCENARIO("queue_list can handle a long queue with a lot of items.")
{
	constexpr unsigned int size = 10000;
	constexpr std::array<api_route, 6> routes = { api_route::fav, api_route::unfav,
		api_route::boost, api_route::unboost, api_route::post, api_route::unpost };

	GIVEN("A bunch of API calls to enqueue and an empty queue_list.")
	{
		test_file queuefile = temporary_file();

		std::vector<api_call> expected;
		expected.reserve(size);

		queue_list actual(queuefile.filename);

		for (unsigned int i = 0; i < size; i++)
		{
			expected.push_back(api_call{ routes[i % routes.size()], "a string" });
			actual.parsed.push_back(api_call{ routes[i % routes.size()], "a string" });
		}

		WHEN("The queue is moved from and destroyed.")
		{
			{
				queue_list newqueue = std::move(actual);
			}

			THEN("Reading the queue again is as expected.")
			{
				queue_list readqueue(queuefile.filename);

				REQUIRE(std::equal(readqueue.parsed.begin(), readqueue.parsed.end(), expected.begin(), expected.end()));
			}
		}

	}
}

SCENARIO("queue_list is a move-only type.")
{
	static_assert(std::is_nothrow_move_constructible<queue_list>::value, "queue_lists should be nothrow move constructible.");
	static_assert(std::is_nothrow_move_assignable<queue_list>::value, "queue_lists should be nothrow move assignable.");
	static_assert(std::is_copy_constructible<queue_list>::value == false, "queue_lists should not be copy constructable.");
	static_assert(std::is_copy_assignable<queue_list>::value == false, "queue_lists should not be copy assignable.");
}