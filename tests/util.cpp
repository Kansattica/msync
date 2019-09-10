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