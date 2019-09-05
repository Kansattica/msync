#include <catch2/catch.hpp>
#include "test_helpers.hpp"

#include <fstream>
#include <array>

#include "../lib/options/user_options.hpp"

SCENARIO("User_options reads from a file when created", "[user_options]")
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

        WHEN("a user_options is created from a filename")
        {
            user_options opt(filepath);

            THEN("the fields are set correctly.")
            {
                REQUIRE(opt.get_option(user_option::accountname).value() == "sometester");
                REQUIRE(opt.get_option(user_option::instanceurl).value() == "website.egg");
            }

            THEN("Empty fields are handled gracefully.")
            {
                REQUIRE_FALSE(opt.get_option(user_option::accesstoken).has_value());
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

        {
            std::ofstream maketest(filepath);

            maketest << "account_name=sometester\n";
            maketest << "instance_url=website.egg\n";
        }

        WHEN("a user_options is created from a filename and a field is changed")
        {
            {
                user_options opt(filepath);
                opt.set_option(user_option::accountname, "someoneelse");
            }

            THEN("a newly created user_options for the same path should see the change.")
            {
                user_options newopt(filepath);
                REQUIRE(newopt.get_option(user_option::accountname).value() == "someoneelse");
                REQUIRE(newopt.get_option(user_option::instanceurl).value() == "website.egg");
                REQUIRE_FALSE(newopt.get_option(user_option::accesstoken).has_value());
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

                opt.set_option(user_option::accountname, "somejerk");
                opt.set_option(user_option::instanceurl, "rude.website");

                THEN("the file is not created until the user_options is destroyed")
                {
                    REQUIRE_FALSE(fs::exists(filepath));
                }
            }

            THEN("The file is created when user_options is destroyed.")
            {
                REQUIRE(fs::exists(filepath));

                const auto lines = read_lines(filepath);

                REQUIRE(lines.size() == 2);
                REQUIRE(lines[0] == "account_name=somejerk");
                REQUIRE(lines[1] == "instance_url=rude.website");
            }
        }
    }
}