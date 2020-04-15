#include <catch2/catch.hpp>

#include "../lib/executablelocation/executable_location.hpp"

SCENARIO("executable_folder returns the same correct path every time.")
{
	GIVEN("The path that executable_folder returns.")
	{
		const auto& folder = executable_folder();

#ifdef _WIN32
		constexpr auto filename = "tests.exe";
#else
		constexpr auto filename = "tests";
#endif

		THEN("tests.exe exists in that folder.")
		{
			REQUIRE(fs::exists(folder / filename));
		}

		THEN("Each subsequent call returns the same path.")
		{
			const auto& also_folder = executable_folder();

			REQUIRE(folder == also_folder);
			REQUIRE(&folder == &also_folder);
		}
	}
}
