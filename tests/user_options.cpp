#include "test_helpers.hpp"
#include <catch2/catch.hpp>

#include <fstream>

#include "../lib/options/user_options.hpp"

SCENARIO("User_options reads from a file when created")
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

SCENARIO("User_options saves changes back to its file")
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

SCENARIO("An empty user_options writes to a file when destroyed")
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

SCENARIO("Get and set options manage string references correctly.")
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

CATCH_REGISTER_ENUM(sync_settings, sync_settings::dont_sync, sync_settings::newest_first, sync_settings::oldest_first);

SCENARIO("The enum overload for get_option works.")
{
    GIVEN("An empty user_options")
    {
        test_file fi{"testfilefriend"};
        user_options opt{"testfilefriend"};

        WHEN("one of the three options that have sync settings is asked for.")
        {
            auto option = GENERATE(user_option::pull_home, user_option::pull_dms, user_option::pull_notifications);
            auto result = opt.get_sync_option(option);

            THEN("the result has the correct default.")
            {
                //newest_first for home, oldest_first for the other two
                if (option == user_option::pull_home)
                {
                    REQUIRE(result == sync_settings::newest_first);
                }
                else
                {
                    REQUIRE(result == sync_settings::oldest_first);
                }
            }
        }
    }

    GIVEN("A user_options with some of the pull options set.")
    {
        const fs::path testfilepath = "testfilefriend";
        test_file fi{testfilepath};
        user_options opt{testfilepath};
        opt.set_option(user_option::pull_home, sync_settings::oldest_first);

        WHEN("one of the three options that have sync settings is asked for.")
        {
            auto option = GENERATE(user_option::pull_home, user_option::pull_dms, user_option::pull_notifications);
            auto result = opt.get_sync_option(option);

            THEN("the result has the correct set or default value.")
            {
                REQUIRE(result == sync_settings::oldest_first);
            }
        }

        WHEN("the user_options is destroyed")
        {
            {
                user_options newopt = std::move(opt);
            }

            THEN("The generated file has the correct option set.")
            {
                auto lines = read_lines(testfilepath);

                REQUIRE(lines.size() == 1);
                REQUIRE(lines[0] == "pull_home=oldest_first");
            }

            AND_WHEN("a new user_options is created from that file")
            {
                user_options neweropt(testfilepath);

                THEN("it has the correct value.")
                {
                    REQUIRE(neweropt.get_sync_option(user_option::pull_home) == sync_settings::oldest_first);
                    REQUIRE(*neweropt.get_option(user_option::pull_home) == "oldest_first");
                }
            }
        }
    }
}