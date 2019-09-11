#include "../lib/util/util.hpp"
#include <catch2/catch.hpp>

#include <string>

using namespace std::string_literals;

SCENARIO("make_api_url correctly concatenates URLs and paths.")
{
    GIVEN("An instance URL and API route")
    {
        auto input = GENERATE(
            std::make_tuple("coolinstance.social"s, "/api/v1/register"s, "https://coolinstance.social/api/v1/register"s),
            std::make_tuple("aplace.egg"s, "/api/v1/howdy"s, "https://aplace.egg/api/v1/howdy"s),
            std::make_tuple("instance.place", "/api/v1/yes", "https://instance.place/api/v1/yes"s));

        WHEN("they're passed to make_api_url")
        {
            std::string result = make_api_url(std::get<0>(input), std::get<1>(input));

            THEN("they're correctly concatenated with the prefix.")
            {
                REQUIRE(result == std::get<2>(input));
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
            std::make_tuple("@_@some-website.comb/"s, "_"s, "some-website.comb"s),
            std::make_tuple("@_@some-website.comb\\"s, "_"s, "some-website.comb"s),
            std::make_tuple("@_@some-website.comb?"s, "_"s, "some-website.comb"s),
            std::make_tuple("@leadingat@boringplace.comb"s, "leadingat"s, "boringplace.comb"s),
            std::make_tuple("@leadingat@http://boringplace.comb"s, "leadingat"s, "boringplace.comb"s),
            std::make_tuple("@leadingat@https://boringplace.comb"s, "leadingat"s, "boringplace.comb"s));

        WHEN(std::get<0>(input) + " is parsed by parse_account_name")
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