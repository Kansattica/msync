#include <catch2/catch.hpp>

#include "../lib/sync/deferred_url_builder.hpp"

SCENARIO("deferred_url_builder correctly constructs and caches URLs.")
{
	GIVEN("A builder with an instance URL.")
	{
		constexpr std::string_view url = "coolwebsite.egg";
		deferred_url_builder builder(url);

		WHEN("A status URL is requested.")
		{
			const auto& status = builder.status_url();

			THEN("The URL is as expected.")
			{
				REQUIRE(status == "https://coolwebsite.egg/api/v1/statuses/");
			}

			THEN("The status URL has a trailing slash.")
			{
				REQUIRE(status.back() == '/');
			}

			AND_WHEN("Another status is requested.")
			{
				const auto& another_status = builder.status_url();

				THEN("The same string is reused.")
				{
					REQUIRE(status == another_status);
					REQUIRE(&status == &another_status);
				}
			}
		}

		WHEN("A media URL is requested.")
		{
			const auto& media = builder.media_url();

			THEN("The URL is as expected.")
			{
				REQUIRE(media == "https://coolwebsite.egg/api/v1/media");
			}

			AND_WHEN("Another media is requested.")
			{
				const auto& another_media = builder.media_url();

				THEN("The same string is reused.")
				{
					REQUIRE(media == another_media);
					REQUIRE(&media == &another_media);
				}
			}
		}

		WHEN("Both a status and media URL are requested.")
		{
			const auto& status = builder.status_url();
			const auto& media = builder.media_url();

			THEN("Both URLS are as expected.")
			{
				REQUIRE(status == "https://coolwebsite.egg/api/v1/statuses/");
				REQUIRE(media == "https://coolwebsite.egg/api/v1/media");
			}

			THEN("The URLs don't equal each other.")
			{
				REQUIRE(status != media);
			}
		}
	}
}