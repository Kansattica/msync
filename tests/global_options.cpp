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
            fs::path accountdir{opts.executable_location};
            accountdir /= Account_Directory;
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
