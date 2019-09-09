#include <catch2/catch.hpp>

#include "../lib/options/options.hpp"

SCENARIO("select_account selects exactly one account.", "[options]")
{
    GIVEN("An empty account_options.")
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
    }
}