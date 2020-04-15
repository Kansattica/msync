#include <catch2/catch.hpp>

#include "../lib/accountdirectory/account_directory.hpp"
#include <constants.hpp>

SCENARIO("account_directory_path returns the same correct path every time.")
{
	GIVEN("The path that account_directory_path returns.")
	{
		const auto& account_dir = account_directory_path();

#ifdef _WIN32
		constexpr auto filename = "tests.exe";
#else
		constexpr auto filename = "tests";
#endif

		THEN("The path ends in in the name of the accounts directory.")
		{
			auto it = --account_dir.end();
			REQUIRE(*it == Account_Directory); 
		}

		THEN("Appending a file name to the path works as expected.")
		{
			const auto with_filename = account_dir / "coolfile";
			auto it = --with_filename.end();
			REQUIRE(*it == "coolfile");
			--it;
			REQUIRE(*it == Account_Directory);
		}

		THEN("The test executable exists in the parent directory.")
		{
			REQUIRE(fs::exists(account_dir.parent_path() / filename));
		}

		THEN("Each subsequent call returns the same path.")
		{
			const auto& also_account_dir = account_directory_path();

			REQUIRE(account_dir == also_account_dir);
			REQUIRE(&account_dir == &also_account_dir);
		}
	}
}
