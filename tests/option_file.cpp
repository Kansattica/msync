#include "test_helpers.hpp"
#include <catch2/catch.hpp>

#include "../lib/options/option_file.hpp"

#include <filesystem.hpp>

#include <fstream>
#include <string>

SCENARIO("option_files save their data when destroyed.")
{
    GIVEN("An option_file with some values.")
    {
        const test_file tf("testfileopt");

        option_file opts(tf.filename);
        opts.parsed["atestoption"] = "coolstuff";
        opts.parsed["test"] = "time";

        REQUIRE(opts.parsed.size() == 2);

        WHEN("an option file is moved from")
        {
            option_file newopts(std::move(opts));

            THEN("no file is written")
            {
                REQUIRE_FALSE(fs::exists(tf.filename));
            }
        }

        WHEN("an option_file is moved from with the move constructor and destroyed")
        {
            {
                option_file newopts(std::move(opts));
            }

            THEN("the file is written")
            {
                REQUIRE(fs::exists(tf.filename));

                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 3);
                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "file_version=1");
                REQUIRE(lines[2] == "test=time");
            }
        }

        WHEN("an option_file is moved from with move assignment and destroyed")
        {
            {
                option_file newopts = std::move(opts);
            }

            THEN("the file gets written")
            {
                REQUIRE(fs::exists(tf.filename));

                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 3);
                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "file_version=1");
                REQUIRE(lines[2] == "test=time");
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
                REQUIRE(fs::exists(tf.filename));

                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "atestoption=coolstuff");
                REQUIRE(lines[1] == "file_version=1");
            }
        }
    }

    GIVEN("An empty option_file.")
    {
        const test_file tf("testfileopt");

		option_file opts{ tf.filename };

        WHEN("The option_file is moved from and destroyed")
        {
            {
				option_file newopts = std::move(opts);
            }

            THEN("A file_version is still added.")
            {
                REQUIRE(fs::exists(tf.filename));

                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 1);
                REQUIRE(lines[0] == "file_version=1");
            }
        }
    }
}

SCENARIO("option_files read data when created.")
{
    GIVEN("An option file on disk with some data.")
    {
        const test_file tf("testfileoptread");

        {
            // .c_str() to make Boost happy
            std::ofstream fout(tf.filename.c_str());
            fout << "somecool=teststuff\n";
            fout << "different=tests\n";
            fout << "imgetting=testy\n";
        }

        WHEN("an option_file is created")
        {
            option_file testfi(tf.filename);

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
                option_file testfi(tf.filename);

                testfi.parsed["anotherentry"] = "foryou";
                testfi.parsed["somecool"] = "isnowthis";
                testfi.parsed.erase("imgetting");

                REQUIRE(testfi.parsed.size() == 3);
            }

            THEN("it saves the new information back to the file.")
            {
                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 4);
                REQUIRE(lines[0] == "anotherentry=foryou");
                REQUIRE(lines[1] == "different=tests");
                REQUIRE(lines[2] == "file_version=1");
                REQUIRE(lines[3] == "somecool=isnowthis");

                AND_THEN("The original file is backed up.")
                {
                    REQUIRE(fs::exists(tf.filenamebak));

                    const auto linesbak = read_lines(tf.filenamebak);

                    REQUIRE(linesbak.size() == 3);
                    REQUIRE(linesbak[0] == "somecool=teststuff");
                    REQUIRE(linesbak[1] == "different=tests");
                    REQUIRE(linesbak[2] == "imgetting=testy");
                }
            }
        }
    }

    GIVEN("An option file on disk with some data, blank lines, and comments.")
    {
        const test_file tf("testfileoptread");

        {
            std::ofstream fout(tf.filename.c_str());
            fout << "somecool=teststuff\n";
            fout << '\n';
            fout << "different=tests\n";
            fout << "#this is a comment\n";
            fout << "imgetting=testy\n";
        }

        WHEN("an option_file is created")
        {
            option_file testfi(tf.filename.c_str());

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
                option_file testfi(tf.filename);

                testfi.parsed["anotherentry"] = "foryou";
                testfi.parsed["somecool"] = "isnowthis";
                testfi.parsed.erase("imgetting");

                REQUIRE(testfi.parsed.size() == 3);
            }

            THEN("it saves the new information back to the file.")
            {
                const auto lines = read_lines(tf.filename);

                REQUIRE(lines.size() == 4);
                REQUIRE(lines[0] == "anotherentry=foryou");
                REQUIRE(lines[1] == "different=tests");
                REQUIRE(lines[2] == "file_version=1");
                REQUIRE(lines[3] == "somecool=isnowthis");

                AND_THEN("The original file is backed up.")
                {
                    REQUIRE(fs::exists(tf.filenamebak));

                    const auto linesbak = read_lines(tf.filenamebak);

                    REQUIRE(linesbak.size() == 5);
                    REQUIRE(linesbak[0] == "somecool=teststuff");
                    REQUIRE(linesbak[1] == "");
                    REQUIRE(linesbak[2] == "different=tests");
                    REQUIRE(linesbak[3] == "#this is a comment");
                    REQUIRE(linesbak[4] == "imgetting=testy");
                }
            }
        }
    }
}

SCENARIO("option_file is nothrow move constructible and assignable, but not copyable.")
{
	static_assert(std::is_nothrow_move_constructible<option_file>::value, "option_files should be nothrow move constructible.");
	static_assert(std::is_nothrow_move_assignable<option_file>::value, "option_files should be nothrow move assignable.");
	static_assert(std::is_copy_constructible<option_file>::value == false, "option_files should not be copy constructible.");
	static_assert(std::is_copy_assignable<option_file>::value == false, "option_files should not be copy assignable.");

}