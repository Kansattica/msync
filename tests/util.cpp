#include "../lib/util/util.hpp"

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#include <catch2/catch.hpp>

#include "test_helpers.hpp"

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
		const auto& input = GENERATE(
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
		const auto& input = GENERATE(
			std::make_tuple("GoddessGrace@goodchristian.website", "GoddessGrace", "goodchristian.website"),
			std::make_tuple("@GoddessGrace@goodchristian.website", "GoddessGrace", "goodchristian.website"),
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
			"@@someinstance",
			"@@some.instance",
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
			std::make_tuple("<p>this is what</p><p>line breaks look like</p>", "this is what\n\nline breaks look like"),
			std::make_tuple("this is what<br><br/>line breaks look like", "this is what\n\nline breaks look like"),
			std::make_tuple("<p>this is a test<br />ignore this</p>", "this is a test\nignore this"),
			std::make_tuple("<p>this is a test<br/><br /><br  />ignore this</p>", "this is a test\n\n\nignore this"),
			std::make_tuple("<p>paragraph tags can have</p> <p>space between</p>\t <p>them.</p>", "paragraph tags can have\n\nspace between\n\nthem."),
			std::make_tuple("<p>paragraph tags<br/>can<br>have</p> <p><br>space between</p>\t <p>them.</p>", "paragraph tags\ncan\nhave\n\n\nspace between\n\nthem."),
			std::make_tuple("<p>hello there</p>", "hello there"),
			std::make_tuple("<p>I'm here</p><div> for you</dove>", "I'm here for you"),
			std::make_tuple("<p>I&apos;m here</p><div> for you</dove>", "I'm here for you"),
			std::make_tuple(" i am com</p>u<to>r", " i am comur"),
			std::make_tuple("this<p>&apos;that", "this'that"),
			std::make_tuple("this<p>&amp;th<div>a</div>t", "this&that"),
			std::make_tuple("&lt;p&gt;hello&lt;/p&gt;", "<p>hello</p>"),
			std::make_tuple("&lt;3", "<3"),
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

	GIVEN("A really long string.")
	{
		const static std::string long_str(70000, 'Q');

		WHEN("The string is stripped as-is.")
		{
			const auto stripped = clean_up_html(long_str);

			THEN("The string is unchanged.")
			{
				REQUIRE(stripped == long_str);
			}

		}

		WHEN("The string is wrapped in paragraph tags and stripped.")
		{
			std::string tostrip = long_str;
			tostrip.reserve(tostrip.size() + 7);
			tostrip.insert(0, "<p>");
			tostrip += "</p>";

			THEN("The stripped string is unchanged.")
			{
				REQUIRE(long_str == clean_up_html(tostrip));
			}

			AND_WHEN("The string is wrapped in some emphasis tags and stripped.")
			{
				tostrip.reserve(tostrip.size() + 9);
				tostrip.insert(0, "<em>");
				tostrip += "</em>";

				THEN("The stripped string is unchanged.")
				{
					REQUIRE(long_str == clean_up_html(tostrip));
				}

				AND_WHEN("Line breaks are added to the beginning, middle, and end of the string before it's stripped.")
				{
					tostrip.insert(0, "<br><br /><br     />");
					tostrip.insert(5000, "<br><br/>");
					tostrip.append("<br       />");

					const auto stripped = clean_up_html(tostrip);

					THEN("The stripped string has newlines in the right places.")
					{
						REQUIRE(stripped[0] == '\n');	
						REQUIRE(stripped[1] == '\n');	
						REQUIRE(stripped[2] == '\n');	
						REQUIRE(stripped[4976] == '\n'); // position shifts when other things get removed.
						REQUIRE(stripped[4977] == '\n');	
						REQUIRE(stripped.back() == '\n');
					}

					THEN("The original string was not modified.")
					{
						REQUIRE(stripped != long_str);
					}

				}
			}


		}

	}
}

struct bulk_replace_test_case
{
	std::string input;
	std::vector<std::pair<std::string_view, std::string_view>> to_replace;
	std::string expected;
};

SCENARIO("bulk_replace_mentions finds and replaces all its arguments in a string in place.")
{
	GIVEN("An input string and a set of substrings to find and replace.")
	{
		auto test_case = GENERATE(
			bulk_replace_test_case{ "", { {"", ""} }, "" },
			bulk_replace_test_case{ "", { {"asdf", "hjkl"} }, "" },
			bulk_replace_test_case{ "@spanky", { {"spanky", "spanky"} }, "@spanky" },
			bulk_replace_test_case{ "spanky @spanky", { {"spanky", "spanky"} }, "spanky @spanky" },
			bulk_replace_test_case{ "@spanky", { {"spanky", "spanky@website.egg"} }, "@spanky@website.egg" },
			bulk_replace_test_case{ "hey, @spanky, what's up", { {"spanky", "spanky@website.egg"} }, "hey, @spanky@website.egg, what's up" },
			bulk_replace_test_case{ "hey, @spanky, what's up. sincerely, @spanky", { {"spanky", "spanky@website.egg"}, {"spanky", "spanky@illegal.egg" } }, "hey, @spanky@website.egg, what's up. sincerely, @spanky@illegal.egg" },
			bulk_replace_test_case{ "@mike @bike @spike heckthread @marge", 
			{ {"mike", "mike@website.egg"}, {"bike", "bike@crime.egg"}, {"spike", "spike"}, {"marge", "marge@in.charge"} },
			"@mike@website.egg @bike@crime.egg @spike heckthread @marge@in.charge" }
			);

		WHEN("The string is replaced on.")
		{
			const auto& replaced = bulk_replace_mentions(test_case.input, test_case.to_replace);

			THEN("The input is replaced in-place.")
			{
				REQUIRE(replaced == test_case.input);
				REQUIRE(&replaced == &test_case.input);
			}

			THEN("The output is as expected.")
			{
				REQUIRE(replaced == test_case.expected);
				REQUIRE(test_case.input == test_case.expected);
			}
		}

	}
}

struct time_test_case
{
	std::string timestamp;
	int year;
	int mon;
	int day;
	int hour;
	int min;
	int sec;
};

SCENARIO("We can correctly parse ISO 8601 timestamps.")
{
	GIVEN("An ISO 8601 timestamp in UTC.")
	{
		// we ignore the decimal portion of the time.
		// notice that the seconds value always gets bumped up,
		// because C++ doesn't have tools for reading fractional seconds.
		const auto test_case = GENERATE(
			time_test_case{ "2020-09-15T18:15:22.938077Z", 120, 8, 15, 18, 15, 23 },
			time_test_case{ "2020-09-15T18:15:00.928077Z", 120, 8, 15, 18, 15, 1 },
			time_test_case{ "2025-11-25T22:02:52.123412Z", 125, 10, 25, 22, 2, 53 }
		);

		WHEN("The timestamp is parsed.")
		{
			const auto timepoint = parse_ISO8601_timestamp(test_case.timestamp);

			THEN("The resulting date and time are correct.")
			{
				const time_t since_epoch = std::chrono::system_clock::to_time_t(timepoint);

				struct tm utctime{};

				wrap_gmtime(&utctime, &since_epoch);

				// https://en.cppreference.com/w/cpp/chrono/c/tm
				REQUIRE(utctime.tm_sec == test_case.sec);
				REQUIRE(utctime.tm_min == test_case.min);
				REQUIRE(utctime.tm_hour == test_case.hour);
				REQUIRE(utctime.tm_mday == test_case.day);
				REQUIRE(utctime.tm_mon == test_case.mon); //january is the 0th month
				REQUIRE(utctime.tm_year == test_case.year); //years since 1900
			}
		}
	}

	GIVEN("An unparseable timestamp.")
	{
		const std::string test_case = GENERATE("asdfadf", "", "2020-09-15T18:15:asasdf", "a string", "123456789");

		WHEN("The timestamp is parsed.")
		{
			const auto timepoint = parse_ISO8601_timestamp(test_case);

			THEN("The returned time point is about a minute in the future.")
			{
				REQUIRE(timepoint >= std::chrono::system_clock::now() + std::chrono::seconds(59));
				REQUIRE(timepoint <= std::chrono::system_clock::now() + std::chrono::seconds(61));
			}
		}
	}
}
