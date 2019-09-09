#include "test_helpers.hpp"
#include <catch2/catch.hpp>
#include <string>

#include "../lib/options/options.hpp"

using namespace std::string_literals;

SCENARIO("select_account selects exactly one account.", "[options]")
{
    GIVEN("An empty account_options")
    {
        options.accounts.clear();

        WHEN("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find a match.");
            }
        }

        WHEN("select_account is given a non-empty string to search on")
        {
            auto account = select_account("coolperson");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find a match.");
            }
        }
    }

    GIVEN("An account_options with one entry")
    {
        const fs::path optiontestfile{"regulartest"};
        test_file fi{optiontestfile};

        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});

        WHEN("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options*>(account));
            }
        }

        WHEN("select_account is given a non-empty string to search on that's a valid prefix")
        {
            auto account = select_account("some");

            THEN("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options*>(account));
            }
        }

        WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
        {
            auto account = select_account("bad");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find a match.");
            }
        }
    }

    GIVEN("An account_options with two entries")
    {
        const fs::path optiontestfile{"regulartest"};
        test_file fi{optiontestfile};
        const fs::path anotheroptiontestfile{"regulartestguy"};
        test_file anotherfi{anotheroptiontestfile};

        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});
        options.accounts.insert({"someotheraccount@place2.egg", user_options(anotheroptiontestfile)});

        for (auto& pair : options.accounts)
        {
            pair.second.set_option(user_option::accountname, std::string(pair.first)); //make a copy because you can't mutate keys, and user_option might mutate
        }

        WHEN("select_account is given an empty string to search on")
        {
            auto account = select_account("");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find an unambiguous match.")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find an unambiguous match.");
            }
        }

        WHEN("select_account is given a non-empty string to search on that's an ambiguous prefix")
        {
            auto account = select_account("some");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find an unambiguous match")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find an unambiguous match.");
            }
        }

        WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix")
        {
            auto account = select_account("someother");

            THEN("a user_options is returned.")
            {
                REQUIRE(std::holds_alternative<const user_options*>(account));
            }

            THEN("the user_options is the correct one.")
            {
                const std::string account_name = *std::get<const user_options*>(account)->get_option(user_option::accountname);
                REQUIRE(account_name == "someotheraccount@place2.egg");
            }
        }

        WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
        {
            auto account = select_account("bad");

            THEN("an error message is returned")
            {
                REQUIRE(std::holds_alternative<const char*>(account));
            }

            THEN("the error is that it couldn't find a match")
            {
                REQUIRE(std::get<const char*>(account) == "Couldn't find a match.");
            }
        }
    }
}

SCENARIO("parse_account_name correctly parses account names into a username and instance URL.")
{
    GIVEN("A correct account name")
    {
        auto input = GENERATE(
            std::make_tuple("GoddessGrace@goodchristian.website"s, "GoddessGrace"s, "goodchristian.website"s),
            std::make_tuple("BestGirl102@good.time.website"s, "BestGirl102"s, "good.time.website"s),
            std::make_tuple("hey_its_m3@internet12.for.egg"s, "hey_its_m3"s, "internet12.for.egg"s),
            std::make_tuple("_@some-website.comb"s, "_"s, "some-website.comb"s),
            std::make_tuple("@_@some-website.comb"s, "_"s, "some-website.comb"s),
            std::make_tuple("@leadingat@boringplace.comb"s, "leadingat"s, "boringplace.comb"s));

        WHEN("it's parsed by parse_account_name")
        {
            auto result = parse_account_name(std::get<0>(input));

            THEN("the parse is good")
            {
                REQUIRE(result.has_value());
            }

            THEN("the username was parsed correctly")
            {
                REQUIRE(result->username == std::get<1>(input));
            }

            THEN("the instance name was parsed correctly")
            {
                REQUIRE(result->instance == std::get<2>(input));
            }
        }
    }

    GIVEN("A bad account name")
    {
        auto input = GENERATE(
            "as;dfhasldfasd"s,
            "badcharacter**@website.com"s,
            "knockitoff@too.dang.many.subdomains"s,
            "noinstance@"s,
            "@nousername"s,
            "what"s,
            "|pipekateer"s,
            ""s,
            ":3@:33.:4"s);

        WHEN("it's parsed by parse_account_name")
        {
            auto result = parse_account_name(input);

            THEN("the parse is bad")
            {
                REQUIRE_FALSE(result.has_value());
            }
        }
    }
}