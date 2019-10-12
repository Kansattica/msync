#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <cpr/cpr.h>


SCENARIO("Can make SSL requests to a site.")
{

	GIVEN("An HTTPS URL")
	{
		auto url = "https://example.com";

		WHEN("a request is made")
		{
			auto response = cpr::Get(cpr::Url{ url });

			THEN("the call is a success.")
			{
				REQUIRE_FALSE(response.error);
				REQUIRE(response.status_code == 200);
			}

			THEN("the body is not empty.")
			{
				REQUIRE_FALSE(response.text.empty());
				REQUIRE(response.text.find("html") != std::string::npos);
			}

		}
	}

}
