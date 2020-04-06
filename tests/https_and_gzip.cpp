#include <catch2/catch.hpp>

#include <cpr/cpr.h>

SCENARIO("Can make SSL requests to a site.")
{
	GIVEN("An HTTPS URL")
	{
		const auto url = GENERATE(as<const char*>{}, "https://example.com", "https://wikipedia.org", "https://mastodon.social");

		CAPTURE(url);

		WHEN("a request is made")
		{
			const auto response = cpr::Get(cpr::Url{ url });

			THEN("the call is a success.")
			{
				INFO("Error: " << response.error.message);
				REQUIRE_FALSE(response.error);
				REQUIRE(response.status_code == 200);
			}

			THEN("the body is not empty.")
			{
				INFO("Error: " << response.error.message);
				REQUIRE_FALSE(response.text.empty());
				REQUIRE(response.text.find("html") != std::string::npos);
			}

			THEN("The response is gzipped.")
			{
				const auto val = response.header.find("Content-Encoding");
				if (val == response.header.end())
				{
					WARN("Response has no Content-Encoding header. msync will work fine, but server responses will not be compressed."); 
				}
				else if (val->second != "gzip")
				{
					WARN("Response from server was not gzipped. Content-Encoding is: " << val->second);
				}
			}

		}
	}

}
