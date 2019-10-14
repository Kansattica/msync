#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <cpr/cpr.h>


SCENARIO("Can make SSL requests to a site.")
{

	GIVEN("An HTTPS URL")
	{
		auto url = GENERATE(as<const char*>{}, "https://example.com", "https://wikipedia.org");

		WHEN("a request is made")
		{
			auto response = cpr::Get(cpr::Url{ url });

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

		}
	}

}
