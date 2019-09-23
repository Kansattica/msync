#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/queue/queue_list.hpp"

#include <filesystem.hpp>

#include <fstream>
#include <string>

SCENARIO("queue_lists save their data when destroyed.", "[queue_list]")
{
    GIVEN("An queue_list with some values.")
    {
        const fs::path testfilename = "testfileopt";
        test_file tf(testfilename);

        queue_list opts(testfilename);
        opts.queued.emplace_back("thingone");
        opts.queued.emplace_back("thingtwo");

        REQUIRE(opts.queued.size() == 2);

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
            opts.queued.pop_front();

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

SCENARIO("queue_lists read data when created.", "[queue_list]")
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
                REQUIRE(testfi.queued.size() == 3);
                REQUIRE(testfi.queued.front() == "firsthing");
                testfi.queued.pop_front();
                REQUIRE(testfi.queued.front() == "secondthing");
                testfi.queued.pop_front();
                REQUIRE(testfi.queued.front() == "thirdthing");
                testfi.queued.pop_front();
            }
        }

        WHEN("an queue_list is opened and modified")
        {
            {
                queue_list testfi(testfilename);

                testfi.queued.pop_front();
                testfi.queued.emplace_back(":) :)");
                testfi.queued.emplace_back(":) :4");
                REQUIRE(testfi.queued.size() == 4);
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