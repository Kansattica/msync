#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/queue/queue_list.hpp"

#include <filesystem.hpp>

#include <fstream>
#include <string>

SCENARIO("queue_lists save their data when destroyed.")
{
    GIVEN("An queue_list with some values.")
    {
        const fs::path testfilename = "testfileopt";
        test_file tf(testfilename);

        queue_list opts(testfilename);
        opts.parsed.emplace_back("thingone");
        opts.parsed.emplace_back("thingtwo");

        REQUIRE(opts.parsed.size() == 2);

        WHEN("an queue_list is moved from")
        {
            queue_list newopts(std::move(opts));

            THEN("no file is written")
            {
                REQUIRE_FALSE(fs::exists(testfilename));
            }
        }

        WHEN("an queue_list is moved from with the move constructor and destroyed")
        {
            {
                queue_list newopts(std::move(opts));
            }

            THEN("the file is written")
            {
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "thingone");
                REQUIRE(lines[1] == "thingtwo");
            }
        }

        WHEN("an queue_list is moved from with move assignment and destroyed")
        {
            {
                queue_list newopts = std::move(opts);
            }

            THEN("the file gets written")
            {
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "thingone");
                REQUIRE(lines[1] == "thingtwo");
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
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 1);
                REQUIRE(lines[0] == "thingtwo");
            }
        }
    }
}

SCENARIO("queue_lists read data when created.")
{
    GIVEN("An queue_list on disk with some data.")
    {
        const fs::path testfilename = "testfileoptread";
        fs::path backupfilename(testfilename);
        backupfilename += ".bak";
        test_file tf(testfilename);

        {
            std::ofstream fout(testfilename);
            fout << "firsthing\n";
            fout << "secondthing\n";
            fout << "thirdthing\n";
        }

        WHEN("an queue_list is created")
        {
            queue_list testfi(testfilename);

            THEN("it has the parsed information from the file.")
            {
                REQUIRE(testfi.parsed.size() == 3);
                REQUIRE(testfi.parsed.front() == "firsthing");
                testfi.parsed.pop_front();
                REQUIRE(testfi.parsed.front() == "secondthing");
                testfi.parsed.pop_front();
                REQUIRE(testfi.parsed.front() == "thirdthing");
                testfi.parsed.pop_front();
            }
        }

        WHEN("an queue_list is opened and modified")
        {
            {
                queue_list testfi(testfilename);

                testfi.parsed.pop_front();
                testfi.parsed.emplace_back(":) :)");
                testfi.parsed.emplace_back(":) :4");
                REQUIRE(testfi.parsed.size() == 4);
            }

            THEN("it saves the new information back to the file.")
            {
                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 4);
                REQUIRE(lines[0] == "secondthing");
                REQUIRE(lines[1] == "thirdthing");
                REQUIRE(lines[2] == ":) :)");
                REQUIRE(lines[3] == ":) :4");

                AND_THEN("The original file is backed up.")
                {
                    REQUIRE(fs::exists(backupfilename));

                    auto linesbak = read_lines(backupfilename);

                    REQUIRE(linesbak.size() == 3);
                    REQUIRE(linesbak[0] == "firsthing");
                    REQUIRE(linesbak[1] == "secondthing");
                    REQUIRE(linesbak[2] == "thirdthing");
                }
            }
        }
    }
}