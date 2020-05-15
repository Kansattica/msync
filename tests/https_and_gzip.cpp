#include <catch2/catch.hpp>

#include <cpr/cpr.h>

SCENARIO("Can make SSL requests to a site.", "[!mayfail]")
{
	GIVEN("An HTTPS URL")
	{
		const auto url = GENERATE(as<const char*>{}, "https://example.com", "https://wikipedia.org", "https://mastodon.social");

		WHEN("a request is made")
		{
			const auto response = cpr::Get(cpr::Url{ url });

			CHECKED_IF ((response.error.code == cpr::ErrorCode::CONNECTION_FAILURE ||
				response.error.code == cpr::ErrorCode::HOST_RESOLUTION_FAILURE))
			{
				CAPTURE(url);
				FAIL("Looks like I couldn't connect to the internet."
					" If you are online, this might just be a temporary hiccup."
					" Feel free to try again."
					"\nThis test checks three different sites."
					" If it only fails for one, it's likely the problem is only with that site, or only temporary, and you should be good.");
				return;
			}

			THEN("the call is a success, the body is not empty, and the response is gzipped.")
			{
				CAPTURE(url);
				INFO("Error: " << response.error.message);
				REQUIRE_FALSE(response.error);
				REQUIRE(response.status_code == 200);

				REQUIRE_FALSE(response.text.empty());
				REQUIRE(response.text.find("html") != std::string::npos);

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
