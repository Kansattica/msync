#include <catch2/catch.hpp>

#include "../lib/sync/read_response.hpp"
#include "../util/util.hpp"

#include <utility>

void statuses_equal(const mastodon_status& expected, const mastodon_status& actual)
{
	REQUIRE(expected.id == actual.id);
	REQUIRE(expected.url == actual.url);
	REQUIRE(expected.content_warning == actual.content_warning);
	REQUIRE(expected.content == actual.content);
	REQUIRE(expected.visibility == actual.visibility);
}

SCENARIO("read_status correctly reads and cleans the relevant fields from a JSON status.")
{
	GIVEN("A json string with the relevant fields.")
	{
		const auto test = GENERATE(
			std::make_pair(R"({"id":"hello", "uri":"https://website.egg/cool", "spoiler_text":"", "content":"hi there", "visibility": "public"})",
				mastodon_status{ "hello", "https://website.egg/cool", "", "hi there", "public" }),
			std::make_pair(R"({"id":"123456789", "uri":"https://website.egg/cool", "spoiler_text":"", "content":"<p>hi there</p>", "visibility": "unlisted"})",
				mastodon_status{ "123456789", "https://website.egg/cool", "", "hi there", "unlisted" }),
			std::make_pair(R"({"id":"22222", "uri":"https://website.egg/cool", "spoiler_text":"", "content":"hi&amp;there", "visibility": "direct"})",
				mastodon_status{ "22222", "https://website.egg/cool", "", "hi&there", "direct" }),
			std::make_pair(R"({"id":"1234567890", "uri":"https://website.egg/cool", "spoiler_text":"ladies", "content":"<p>hi there</p>", "visibility": "unlisted"})",
				mastodon_status{ "1234567890", "https://website.egg/cool", "ladies", "hi there", "unlisted" })
		);

		WHEN("The string is read.")
		{
			const auto result = read_status(std::get<0>(test));

			THEN("The result is as expected.")
			{
				statuses_equal(std::get<1>(test), result);
			}
		}
	}
}

SCENARIO("read_upload_id correctly reads the ID from a JSON status.")
{
	GIVEN("A json string with the relevant fields.")
	{
		const auto test = GENERATE(
			std::make_pair(R"({"id":"hello", "uri":"https://website.egg/cool"})",
				"hello"),
			std::make_pair(R"({"id":"123456789", "uri":"https://website.egg/cool"})",
				"123456789"),
			std::make_pair(R"({"id":"22222", "uri":"https://website.egg/cool"})",
				"22222")
		);

		WHEN("The string is read.")
		{
			const auto result = read_upload_id(std::get<0>(test));

			THEN("The result is as expected.")
			{
				REQUIRE(std::get<1>(test) == result);
			}
		}
	}
}

