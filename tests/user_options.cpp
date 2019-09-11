#include "test_helpers.hpp"
#include <catch2/catch.hpp>

#include <array>
#include <fstream>


#include "../lib/options/user_options.hpp"

SCENARIO("User_options reads from a file when created", "[user_options]")
{
    GIVEN("A file on disk with some properly formatted data")
    {
        const fs::path filepath("someuser.test");
        test_file fi(filepath);
        const fs::path backup("someuser.test.bak");

        {
            std::ofstream maketest(filepath);

            maketest << "account_name=sometester\n";
            maketest << "instance_url=website.egg\n";
        }

        WHEN("a user_options is created from a filename")
        {
            user_options opt(filepath);

            THEN("the fields are set correctly.")
            {
                REQUIRE(*opt.get_option(user_option::account_name) == "sometester");
                REQUIRE(*opt.get_option(user_option::instance_url) == "website.egg");
            }

            THEN("empty fields are handled gracefully.")
            {
                REQUIRE_FALSE(opt.get_option(user_option::access_token) != nullptr);
            }

            THEN("No .bak file is created.")
            {
                REQUIRE_FALSE(fs::exists(backup));
            }
        }
    }
}

SCENARIO("User_options saves changes back to its file", "[user_options]")
{
    GIVEN("A file on disk with some properly formatted data")
    {
        const fs::path filepath("someuser.test");
        test_file fi(filepath);

        fs::path backup(filepath);
        backup += ".bak";

        {
            std::ofstream maketest(filepath);

            maketest << "account_name=sometester\n";
            maketest << "instance_url=website.egg\n";
        }

        WHEN("a user_options is created from a filename and a field is changed")
        {
            {
                user_options opt(filepath);
                opt.set_option(user_option::account_name, "someoneelse");
            }

            THEN("a newly created user_options for the same path should see the change.")
            {
                user_options newopt(filepath);
                REQUIRE(*newopt.get_option(user_option::account_name) == "someoneelse");
                REQUIRE(*newopt.get_option(user_option::instance_url) == "website.egg");
                REQUIRE(newopt.get_option(user_option::access_token) == nullptr);
            }

            THEN("a .bak file with the original information should be created.")
            {
                REQUIRE(fs::exists(backup));

                auto lines = read_lines(backup);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "account_name=sometester");
                REQUIRE(lines[1] == "instance_url=website.egg");
            }
        }
    }
}

SCENARIO("An empty user_options writes to a file when destroyed", "[user_options]")
{
    GIVEN("A path with no file on disk")
    {
        const fs::path filepath("someuser.test");
        test_file fi(filepath);

        WHEN("a user_options is created with that path and modified")
        {
            {
                user_options opt(filepath);

                opt.set_option(user_option::account_name, "somejerk");
                opt.set_option(user_option::instance_url, "rude.website");
                opt.set_option(user_option::pull_dms, sync_settings::newest_first);

                THEN("the file is not created until the user_options is destroyed")
                {
                    REQUIRE_FALSE(fs::exists(filepath));
                }
            }

            THEN("The file is created when user_options is destroyed.")
            {
                REQUIRE(fs::exists(filepath));

                const auto lines = read_lines(filepath);

                REQUIRE(lines.size() == 3);
                REQUIRE(lines[0] == "account_name=somejerk");
                REQUIRE(lines[1] == "instance_url=rude.website");
                REQUIRE(lines[2] == "pull_dms=newest_first");
            }

            THEN("No .bak file is created.")
            {
                fs::path backup(filepath);
                backup += ".bak";
                REQUIRE_FALSE(fs::exists(backup));
            }
        }
    }
}

SCENARIO("Get and set options manage string references correctly.", "[user_options]")
{
    GIVEN("A file on disk with some properly formatted data")
    {
        const fs::path filepath("someuser.test");
        test_file fi(filepath);

        {
            std::ofstream maketest(filepath);

            maketest << "account_name=sometester\n";
            maketest << "instance_url=website.egg\n";
        }

        WHEN("an option that exists in the dictionary is requested")
        {
            user_options opt(filepath);
            auto val = opt.get_option(user_option::account_name);

            THEN("the value is present.")
            {
                REQUIRE(val != nullptr);
            }

            THEN("the value is correct.")
            {
                REQUIRE(*val == "sometester");
            }

            AND_WHEN("the option is modified")
            {
                opt.set_option(user_option::account_name, "someotherguy");

                THEN("the change is reflected in the original value")
                {
                    REQUIRE(*val == "someotherguy");
                }
            }
        }
    }
}