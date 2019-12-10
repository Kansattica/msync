#include "../lib/util/util.hpp"
#include <catch2/catch.hpp>

#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <sstream>

using namespace std::string_view_literals;

SCENARIO("make_api_url correctly concatenates URLs and paths.")
{
    GIVEN("An instance URL and API route")
    {
        const auto input = GENERATE(
            std::make_tuple("coolinstance.social", "/api/v1/register", "https://coolinstance.social/api/v1/register"),
            std::make_tuple("aplace.egg", "/api/v1/howdy", "https://aplace.egg/api/v1/howdy"),
            std::make_tuple("instance.place", "/api/v1/yes", "https://instance.place/api/v1/yes"));

        WHEN("they're passed to make_api_url")
        {
            const std::string result = make_api_url(std::get<0>(input), std::get<1>(input));

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
        const auto input = GENERATE(
            std::make_tuple("GoddessGrace@goodchristian.website", "GoddessGrace", "goodchristian.website"),
            std::make_tuple("BestGirl102@good.time.website", "BestGirl102", "good.time.website"),
            std::make_tuple("hey_its_m3@internet12.for.egg", "hey_its_m3", "internet12.for.egg"),
            std::make_tuple("_@some-website.comb", "_", "some-website.comb"),
            std::make_tuple("@_@some-website.comb", "_", "some-website.comb"),
            std::make_tuple("@_@some-website.comb/", "_", "some-website.comb"),
            std::make_tuple("@_@some-website.comb\\", "_", "some-website.comb"),
            std::make_tuple("@_@some-website.comb?", "_", "some-website.comb"),
            std::make_tuple("@leadingat@boringplace.comb", "leadingat", "boringplace.comb"),
            std::make_tuple("@leadingat@http://boringplace.comb", "leadingat", "boringplace.comb"),
            std::make_tuple("@leadingat@https://boringplace.comb", "leadingat", "boringplace.comb"));

        WHEN("an account name is parsed by parse_account_name")
        {
            const auto result = parse_account_name(std::get<0>(input));

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
		const auto input = GENERATE(as<const char*>{}, //msvc won't build without this
            "as;dfhasldfasd",
            "badcharacter**@website.com",
            "knockitoff@too.dang.many.subdomains",
            "noinstance@",
            "@nousername",
            "what",
            "|pipekateer",
            "",
            ":3@:33.:4");

        WHEN("it's parsed by parse_account_name")
        {
            const auto result = parse_account_name(input);

            THEN("the parse is bad")
            {
                REQUIRE_FALSE(result.has_value());
            }
        }
    }
}

SCENARIO("split_string correctly splits strings")
{
	GIVEN("Some strings with the delimiter and nothing extra at the end.")
	{
		const auto input = GENERATE(
			std::make_tuple("a,delimited,string", ',', std::vector<std::string_view>{"a", "delimited", "string"}),
			std::make_tuple("a,delimited,string", ';', std::vector<std::string_view>{"a,delimited,string"}),
			std::make_tuple("a;delimited;string", ';', std::vector<std::string_view>{"a", "delimited", "string"}),
			std::make_tuple("just;one", ';', std::vector<std::string_view>{"just", "one"}),
			std::make_tuple("what,about,a,really,long,one;huh", ',', std::vector<std::string_view>{"what", "about", "a", "really", "long", "one;huh"}));

		WHEN("the string is split, ignoring empty splits.")
		{
			const auto result = split_string<false>(std::get<0>(input), std::get<1>(input));

			THEN("the result is what we expected.")
			{
				REQUIRE(result == std::get<2>(input));
			}
		}

		WHEN("the string is split, including empty splits.")
		{
			const auto result = split_string<true>(std::get<0>(input), std::get<1>(input));

			THEN("the result is what we expected.")
			{
				REQUIRE(result == std::get<2>(input));
			}
		}
	}

	GIVEN("Some strings with the delimiter at the end.")
	{
		const auto input = GENERATE(
			std::make_tuple("a,delimited,string,", ',', std::vector<std::string_view>{"a", "delimited", "string"}),
			std::make_tuple("a,delimited,string,,,", ',', std::vector<std::string_view>{"a", "delimited", "string"}),
			std::make_tuple(",", ',', std::vector<std::string_view>{}),
			std::make_tuple("a,delimited,string;", ';', std::vector<std::string_view>{"a,delimited,string"}),
			std::make_tuple("a;delimited;string;", ';', std::vector<std::string_view>{"a", "delimited", "string"}),
			std::make_tuple("just;one;", ';', std::vector<std::string_view>{"just", "one"}),
			std::make_tuple("what,about,a,really,long,one;huh,", ',', std::vector<std::string_view>{"what", "about", "a", "really", "long", "one;huh"}));

		WHEN("the string is split with empty strings turned off.")
		{
			const auto result = split_string(std::get<0>(input), std::get<1>(input));

			THEN("the result is what we expected.")
			{
				REQUIRE(result == std::get<2>(input));
			}
		}
	}

	GIVEN("Some strings with multiple consecutive delimiters.")
	{
		const auto input = GENERATE(
			std::make_tuple("a,,,delimited,,string,", ',', std::vector<std::string_view>{"a", "delimited", "string"}, std::vector<std::string_view>{"a", "", "", "delimited", "", "string"}),
			std::make_tuple("a,del,,imited,,string,,,", ',', std::vector<std::string_view>{"a", "del", "imited", "string"}, std::vector<std::string_view>{"a", "del", "", "imited", "", "string", "", ""}),
			std::make_tuple(",,,,", ',', std::vector<std::string_view>{}, std::vector<std::string_view>{"", "", "", ""}),
			std::make_tuple(";a,delimited,string;;", ';', std::vector<std::string_view>{"a,delimited,string"}, std::vector<std::string_view>{"", "a,delimited,string", ""}),
			std::make_tuple("a;;;;delimited;;;string;;;;;", ';', std::vector<std::string_view>{"a", "delimited", "string"}, std::vector<std::string_view>{"a", "","","", "delimited", "","", "string", "","","",""}),
			std::make_tuple("just;;one;", ';', std::vector<std::string_view>{"just", "one"}, std::vector<std::string_view>{"just", "", "one"}),
			std::make_tuple("what,about,,,,a,,really,,long,,,,,one;huh,", ',', std::vector<std::string_view>{"what", "about", "a", "really", "long", "one;huh"}, std::vector<std::string_view>{"what", "about", "", "", "", "a", "", "really", "", "long", "", "", "", "", "one;huh"})
		);

		WHEN("the string is split with empty strings turned off.")
		{
			const auto result = split_string<false>(std::get<0>(input), std::get<1>(input));

			THEN("the result is what we expected.")
			{
				REQUIRE(result == std::get<2>(input));
			}
		}

		WHEN("the string is split with empty strings turned on.")
		{
			const auto result = split_string<true>(std::get<0>(input), std::get<1>(input));

			THEN("the result is what we expected.")
			{
				REQUIRE(result == std::get<3>(input));
			}
		}
	}

	GIVEN("The empty string")
	{
		const auto input = "";

		WHEN("the empty string is split, ignoring empty splits")
		{
			const auto result = split_string(input, ',');

			THEN("we get an empty vector back.")
			{
				REQUIRE(result.size() == 0);
			}
		}

		WHEN("the empty string is split, not ignoring empty splits")
		{
			const auto result = split_string<true>(input, ',');

			THEN("we get an empty vector back.")
			{
				REQUIRE(result.size() == 0);
			}
		}
	}
}

SCENARIO("join_iterable correctly does that.")
{
	GIVEN("An iterable to join")
	{
		const auto testcase = GENERATE(
			std::make_tuple(std::vector<std::string_view>{"this","is","some","stuff"}, ";"sv, "this;is;some;stuff"sv),
			std::make_tuple(std::vector<std::string_view>{"this","is","some","stuff"}, "_"sv, "this_is_some_stuff"sv),
			std::make_tuple(std::vector<std::string_view>{"this","is","not","some","other","stuff"}, " or "sv, "this or is or not or some or other or stuff"sv),
			std::make_tuple(std::vector<std::string_view>{"includes", "anempty", "", "string"}, "!"sv, "includes!anempty!!string"sv),
			std::make_tuple(std::vector<std::string_view>{}, "_"sv, ""sv),
			std::make_tuple(std::vector<std::string_view>{"hello"}, "_"sv, "hello"sv)
		);

		WHEN("the iterable is joined")
		{
			std::stringstream ss;
			join_iterable(std::get<0>(testcase).begin(), std::get<0>(testcase).end(), std::get<1>(testcase), ss);

			THEN("the result is as expected.")
			{
				REQUIRE(ss.str() == std::get<2>(testcase));
			}
		}
	}
}

SCENARIO("split_string and join_iterable are inverses.")
{
	GIVEN("A string to split and then join on a char")
	{
		const auto testcase = GENERATE(
			std::make_tuple("this,is,a,string", ','),
			std::make_tuple("ladies:hello", ':'),
			std::make_tuple("what,about;this,one", ','),
			std::make_tuple("what,about;this,,one", ','),
			std::make_tuple("what,about;this,one", ';'),
			std::make_tuple("", ';')
		);

		WHEN("the string is split and then joined")
		{
			const auto split = split_string<true>(std::get<0>(testcase), std::get<1>(testcase));
			
			std::stringstream ss;
			join_iterable(split.begin(), split.end(), std::get<1>(testcase), ss);

			THEN("the result is as expected.")
			{
				REQUIRE(ss.str() == std::get<0>(testcase));
			}
		}
	}

	GIVEN("A string to join and then split on a char")
	{
		const auto testcase = GENERATE(
			std::make_tuple(std::vector<std::string_view>{"this","is","some","stuff"}, ';'),
			std::make_tuple(std::vector<std::string_view>{"this","is","some","stuff"}, '_'),
			std::make_tuple(std::vector<std::string_view>{"includes", "anempty", "", "string"}, '!'),
			std::make_tuple(std::vector<std::string_view>{}, '_'),
			std::make_tuple(std::vector<std::string_view>{"hello"}, '_')
		);

		WHEN("the iterable is joined and then split")
		{
			std::stringstream ss;
			join_iterable(std::get<0>(testcase).begin(), std::get<0>(testcase).end(), std::get<1>(testcase), ss);
			
			// can't return stringviews into ss.str(), it stops existing after the call
			const auto joined = std::string{ ss.str() };

			const auto split = split_string<true>(joined, std::get<1>(testcase));

			THEN("the result is as expected.")
			{
				REQUIRE(split == std::get<0>(testcase));
			}
		}
	}
}

SCENARIO("clean_up_html removes HTML tags and entities.")
{
	GIVEN("A string with some HTML tags and/or entities in it.")
	{
		const auto tostrip = GENERATE(
			std::make_tuple("<p>this is what</p><p>line breaks look like</p>", "this is what\nline breaks look like"),
			std::make_tuple("this is what<br><br/>line breaks look like", "this is what\n\nline breaks look like"),
			std::make_tuple("<p>this is a test<br />ignore this</p>", "this is a test\nignore this"),
			std::make_tuple("<p>hello there</p>", "hello there"),
			std::make_tuple("<p>I'm here</p><div> for you</dove>", "I'm here for you"),
			std::make_tuple("<p>I&apos;m here</p><div> for you</dove>", "I'm here for you"),
			std::make_tuple(" i am com</p>u<to>r", " i am comur"),
			std::make_tuple("this<p>&apos;that", "this'that"),
			std::make_tuple("this<p>&amp;th<div>a</div>t", "this&that"),
			std::make_tuple("&lt;p&gt;hello&lt;/p&gt;", "<p>hello</p>"),
			std::make_tuple("<p>look at my :custom_emojo:</p>", "look at my :custom_emojo:"),
			std::make_tuple("&hearts;&hearts;&hearts;&hearts;&hearts;&hearts;", "♥♥♥♥♥♥")
		);

		WHEN("the HTML is cleaned up")
		{
			const auto stripped = clean_up_html(std::get<0>(tostrip));

			THEN("the entities are decoded and has no HTML tags.")
			{
				REQUIRE(stripped == std::get<1>(tostrip));
			}
		}
	}

	GIVEN("A string with no HTML tags in it.")
	{
		const auto tostrip = GENERATE(as<const char*>{}, //msvc won't build without this
			"hello",
			"<:)",
			">:(",
			">:3",
			"This is a regular sentence.      \t\n",
			";>",
			":>",
			">:>",
			"&as",
			"&;&;&;><as",
			"",
			" \t\t\n",
			"",
			"<hatml  /"
		);

		WHEN("the HTML is stripped")
		{
			const auto stripped = clean_up_html(tostrip);

			THEN("the result is unchanged.")
			{
				REQUIRE(stripped == tostrip);
			}
		}
	}
}