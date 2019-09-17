#include "test_helpers.hpp"
#include <catch2/catch.hpp>

#include "../lib/options/option_file.hpp"

#include <filesystem.hpp>

#include <fstream>
#include <string>

SCENARIO("option_files save their data when destroyed.", "[option_file]")
{
    GIVEN("An option_file with some values.")
    {
        const fs::path testfilename = "testfileopt";
        test_file tf(testfilename);

        option_file opts(testfilename);
        opts.parsed["atestoption"] = "coolstuff";
        opts.parsed["test"] = "time";

        REQUIRE(opts.parsed.size() == 2);

        WHEN("an option file is moved from")
        {
            option_file newopts(std::move(opts));

            THEN("no file is written")
            {
                REQUIRE_FALSE(fs::exists(testfilename));
            }
        }

        WHEN("an option_file is moved from with the move constructor and destroyed")
        {
            {
                option_file newopts(std::move(opts));
            }

            THEN("the file is written")
            {
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "test=time");
            }
        }

        WHEN("an option_file is moved from with move assignment and destroyed")
        {
            {
                option_file newopts = std::move(opts);
            }

            THEN("the file gets written")
            {
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "test=time");
            }
        }

        WHEN("an option's value is set to an empty string and destroyed")
        {
            opts.parsed["test"] = "";

            {
                option_file newopts = std::move(opts);
            }

            THEN("the file gets written without the deleted options.")
            {
                REQUIRE(fs::exists(testfilename));

                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 1);
                REQUIRE(lines[0] == "atestoption=coolstuff");
            }
        }
    }
}

SCENARIO("option_files read data when created.", "[option_file]")
{
    GIVEN("An option file on disk with some data.")
    {
        const fs::path testfilename = "testfileoptread";
        fs::path backupfilename(testfilename);
        backupfilename += ".bak";
        test_file tf(testfilename);

        {
            std::ofstream fout(testfilename);
            fout << "somecool=teststuff\n";
            fout << "different=tests\n";
            fout << "imgetting=testy\n";
        }

        WHEN("an option_file is created")
        {
            option_file testfi(testfilename);

            THEN("it has the parsed information from the file.")
            {
                REQUIRE(testfi.parsed.size() == 3);
                REQUIRE(testfi.parsed["somecool"] == "teststuff");
                REQUIRE(testfi.parsed["different"] == "tests");
                REQUIRE(testfi.parsed["imgetting"] == "testy");
            }
        }

        WHEN("an option_file is opened and modified")
        {
            {
                option_file testfi(testfilename);

                testfi.parsed["anotherentry"] = "foryou";
                testfi.parsed["somecool"] = "isnowthis";
                testfi.parsed.erase("imgetting");

                REQUIRE(testfi.parsed.size() == 3);
            }

            THEN("it saves the new information back to the file.")
            {
                auto lines = read_lines(testfilename);

                REQUIRE(lines.size() == 3);
                REQUIRE(lines[0] == "anotherentry=foryou");
                REQUIRE(lines[1] == "different=tests");
                REQUIRE(lines[2] == "somecool=isnowthis");

                AND_THEN("The original file is backed up.")
                {
                    REQUIRE(fs::exists(backupfilename));

                    auto linesbak = read_lines(backupfilename);

                    REQUIRE(linesbak.size() == 3);
                    REQUIRE(linesbak[0] == "somecool=teststuff");
                    REQUIRE(linesbak[1] == "different=tests");
                    REQUIRE(linesbak[2] == "imgetting=testy");
                }
            }
        }
    }
}