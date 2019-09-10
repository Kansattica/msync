#include <catch2/catch.hpp>
#include "test_helpers.hpp"
#include <constants.hpp>

#include "../lib/options/global_options.hpp"

SCENARIO ("Both paths we care about are directories.")
{
    GIVEN("A global_options object")
    {
        global_options opts;

        WHEN ("the object is created")
        {
            THEN ("current_working_directory is actually a directory.")
            {
                REQUIRE(fs::is_directory(opts.current_working_directory));
            }

            THEN ("executable_location is actually a directory.")
            {
                REQUIRE(fs::is_directory(opts.executable_location));
            }
        }
    }
}

SCENARIO ("add_new_account correctly handles input.")
{
    GIVEN("A global_options object")
    {
        global_options opts;

        WHEN("add_new_account is called on it with a valid username")
        {
            const fs::path accountdir = opts.executable_location / Account_Directory;
            test_file fi{accountdir};

            fs::path userfile{accountdir};
            userfile /= "coolguy@website.com";
            userfile /= User_Options_Filename;

            user_options& added = opts.add_new_account("coolguy@website.com");

            THEN("the accounts directory is created.")
            {
               REQUIRE(fs::exists(accountdir));
               REQUIRE(fs::is_directory(accountdir));
            }

            THEN("no file is created immediately.")
            {
                REQUIRE_FALSE(fs::exists(userfile));
            }

            AND_WHEN("the accounts are cleared")
            {
                opts.accounts.clear();

                THEN("a file is created at the correct place.")
                {
                    fs::path userfile{accountdir};
                    userfile /= "coolguy@website.com";
                    userfile /= User_Options_Filename;
                    REQUIRE(fs::exists(userfile));
                }
            }
        }
    }
}

SCENARIO ("read_accounts correctly fills global_options on construction.")
{
    GIVEN("A global_options with some values added to it, destroyed, and then a new one created.")
    {
        fs::path accountdir;
        {
            global_options opt;
            accountdir = opt.executable_location / Account_Directory;

            {
                test_file fi{accountdir};
            }

            user_options& added = opt.add_new_account("coolaccount@website.com");
            added.set_option(user_option::accountname, "coolaccount");
            added.set_option(user_option::instanceurl, "website.com");
            user_options& alsoadded = opt.add_new_account("evencooleraccount@wedsize.egg");
            alsoadded.set_option(user_option::accountname, "evencooleraccount");
            alsoadded.set_option(user_option::instanceurl, "wedsize.egg");
        }

        global_options opt;

        WHEN ("a given account is looked up")
        {
            auto found = opt.accounts.find("coolaccount@website.com");

            THEN ("something was found")
            {
                REQUIRE(found != opt.accounts.end());
            }

            THEN ("it was what we expected")
            {
                REQUIRE(found->first == "coolaccount@website.com");
                REQUIRE(*found->second.get_option(user_option::accountname) == "coolaccount");
                REQUIRE(*found->second.get_option(user_option::instanceurl) == "website.com");
            }
        }
    }
}
