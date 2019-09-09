#include <catch2/catch.hpp>
#include "test_helpers.hpp"

#include "../lib/options/options.hpp"


SCENARIO("select_account selects exactly one account.", "[options]")
{
    GIVEN("An empty account_options")
    {
        options.accounts.clear();

        WHEN ("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find a match.");
            }
        }

        WHEN ("select_account is given a non-empty string to search on")
        {
            auto account = select_account("coolperson");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find a match.");
            }
        }
    }

    GIVEN("An account_options with one entry")
    {
        const fs::path optiontestfile {"regulartest"};
        test_file fi {optiontestfile};

        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});

        WHEN ("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN ("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options* const>(account));
            }
        }

        WHEN ("select_account is given a non-empty string to search on that's a valid prefix")
        {
            auto account = select_account("some");

            THEN ("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options* const>(account));
            }
        }

        WHEN ("select_account is given a non-empty string to search on that's not a valid prefix")
        {
            auto account = select_account("bad");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find a match.");
            }
        }
    }

    GIVEN("An account_options with two entries")
    {
        const fs::path optiontestfile {"regulartest"};
        test_file fi {optiontestfile};
        const fs::path anotheroptiontestfile {"regulartestguy"};
        test_file anotherfi {anotheroptiontestfile};

        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});
        options.accounts.insert({"someotheraccount@place2.egg", user_options(optiontestfile)});

        for (auto &pair : options.accounts)
        {
            pair.second.set_option(user_option::accountname, std::string(pair.first)); //make a copy because you can't mutate keys, and user_option might mutate
        }

        WHEN ("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find an unambiguous match.")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find an unambiguous match.");
            }
        }

        WHEN ("select_account is given a non-empty string to search on that's an ambiguous prefix")
        {
            auto account = select_account("some");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find an unambiguous match")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find an unambiguous match.");
            }
        }

        WHEN ("select_account is given a non-empty string to search on that's an unambiguous prefix")
        {
            auto account = select_account("someother");

            THEN ("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options* const>(account));
            }

            THEN ("the user_options is the correct one.")
            {
                const std::string account_name = *std::get<const user_options* const>(account)->get_option(user_option::accountname);
                REQUIRE(account_name == "someotheraccount@place2.egg");
            }
        }

        WHEN ("select_account is given a non-empty string to search on that's not a valid prefix")
        {
            auto account = select_account("bad");

            THEN ("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const std::string>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const std::string>(account) == "Couldn't find a match.");
            }
        }
    }
}