#include <catch2/catch.hpp>

#include "../lib/options/global_options.hpp"

SCENARIO ("Both paths we care about are directories.")
{
    GIVEN("A global_options object")
    {
        global_options opts;

        WHEN ("the object is created")
        {
            THEN ("current_working_directory is actually a directory.")
            {
                REQUIRE(fs::is_directory(opts.current_working_directory));
            }

            THEN ("executable_location is actually a directory.")
            {
                REQUIRE(fs::is_directory(opts.executable_location));
            }
        }
    }
}

