#include <catch2/catch.hpp>

#include "../lib/options/option_file.hpp"

#include "../lib/options/filesystem.hpp"

#include <fstream>
#include <string>
#include <array>

SCENARIO("option_files save their data when destroyed.", "[option_file]")
{
    GIVEN("An option file with some values.")
    {
        const fs::path testfilename = "testfileopt";
        if (fs::exists(testfilename))
            fs::remove(testfilename);

        option_file opts(testfilename);
        opts.parsed_options["atestoption"] = "coolstuff";
        opts.parsed_options["test"] = "time";

        REQUIRE(opts.parsed_options.size() == 2);

        WHEN("an option file is moved from")
        {
            option_file newopts(std::move(opts));

            THEN("no file is written")
            {
                REQUIRE(!fs::exists(testfilename));
            }
        }

        WHEN("an option file is moved from with the move constructor and destroyed")
        {
            {
                option_file newopts(std::move(opts));
            }

            THEN("the file is written")
            {
                REQUIRE(fs::exists(testfilename));

                std::array<std::string, 2> lines;
                std::ifstream fin(testfilename);

                std::getline(fin, lines[0]);
                std::getline(fin, lines[1]);

                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "test=time");
            }
        }

        WHEN("an option file is moved from with move assignment and destroyed")
        {
            {
                option_file newopts = std::move(opts);
            }

            THEN("the file is written")
            {
                REQUIRE(fs::exists(testfilename));

                std::array<std::string, 2> lines;
                std::ifstream fin(testfilename);

                std::getline(fin, lines[0]);
                std::getline(fin, lines[1]);

                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "test=time");
            }
        }
    }
}

SCENARIO("option_files read data when created.", "[option_file]")
{
    GIVEN("An option file with some data.")
    {
        const fs::path testfilename = "testfileoptread";
        fs::path backupfilename(testfilename);
        backupfilename += ".bak";
        if (fs::exists(testfilename))
            fs::remove(testfilename);

        if (fs::exists(backupfilename))
            fs::remove(backupfilename);

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
                REQUIRE(testfi.parsed_options.size() == 3);
                REQUIRE(testfi.parsed_options["somecool"] == "teststuff");
                REQUIRE(testfi.parsed_options["different"] == "tests");
                REQUIRE(testfi.parsed_options["imgetting"] == "testy");
            }
        }

        WHEN("an option_file is opened and modified")
        {
            {
                option_file testfi(testfilename);

                testfi.parsed_options["anotherentry"] = "foryou";
                testfi.parsed_options["somecool"] = "isnowthis";
                testfi.parsed_options.erase("imgetting");

                REQUIRE(testfi.parsed_options.size() == 3);
            }

            THEN("it saves the new information back to the file.")
            {
                std::array<std::string, 3> lines;
                std::ifstream fin(testfilename);

                std::getline(fin, lines[0]);
                std::getline(fin, lines[1]);
                std::getline(fin, lines[2]);

                REQUIRE(lines[0] == "anotherentry=foryou");
                REQUIRE(lines[1] == "different=tests");
                REQUIRE(lines[2] == "somecool=isnowthis");

                AND_THEN("The original file is backed up.")
                {
                    REQUIRE(fs::exists(backupfilename));

                    std::array<std::string, 3> lines;
                    std::ifstream fin(backupfilename);

                    std::getline(fin, lines[0]);
                    std::getline(fin, lines[1]);
                    std::getline(fin, lines[2]);

                    REQUIRE(lines[0] == "somecool=teststuff");
                    REQUIRE(lines[1] == "different=tests");
                    REQUIRE(lines[2] == "imgetting=testy");
                }
            }
        }
    }
}