#include <catch2/catch.hpp>

#include <constants.hpp>

#if !defined(_WIN32) && defined(MSYNC_USER_CONFIG)
	#include <cstdlib>
	#include "test_helpers.hpp"
	#include <string>
	#define MSYNC_TESTING
#endif

#include "../lib/accountdirectory/account_directory.hpp"

SCENARIO("account_directory_path returns the same correct path every time.")
{
	GIVEN("The path that account_directory_path returns.")
	{
		const auto& account_dir = account_directory_path();

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

#ifndef MSYNC_USER_CONFIG
		THEN("The test executable exists in the parent directory.")
		{
			constexpr auto filename = 
#ifdef _WIN32
			"tests.exe";
#else
			"tests";
#endif
			REQUIRE(fs::exists(account_dir.parent_path() / filename));
		}
#endif

		THEN("Each subsequent call returns the same path.")
		{
			const auto& also_account_dir = account_directory_path();

			REQUIRE(account_dir == also_account_dir);
			REQUIRE(&account_dir == &also_account_dir);
		}
	}
}

#ifdef MSYNC_USER_CONFIG
SCENARIO("The account directory locator respects MSYNC_USER_CONFIG.")
{
	#ifdef _WIN32
	GIVEN("The account directory path.")
	{
		const auto account_dir = account_directory_path();

		THEN("On Windows, the path ends with AppData/Local/msync/msync_accounts.")
		{
			auto path_iterator = account_dir.end();
			REQUIRE(*(--path_iterator) == Account_Directory);
			REQUIRE(*(--path_iterator) == "msync");
			REQUIRE(*(--path_iterator) == "Local");
			REQUIRE(*(--path_iterator) == "AppData");
		}
	}
	#else // linux and OSX
	GIVEN("An environment where XDG_CONFIG_HOME can be checked.")
	{
		constexpr auto xdg_home = "XDG_CONFIG_HOME";
		// the string that getenv returns changes when you call setenv, so save it off
		const auto oldhome = getenv(xdg_home);
		const std::string old_home_val = oldhome == nullptr ? "" : oldhome;

		WHEN("XDG_CONFIG_HOME is unset.")
		{
			REQUIRE(unsetenv(xdg_home) == 0);
			const auto account_dir = account_directory_path_uncached();
			THEN("The path is as expected, ending in .config/msync/msync_accounts.")
			{
				const auto expectedpath = fs::path { getenv("HOME") } / ".config" / "msync" / Account_Directory;
				REQUIRE(account_dir == expectedpath);
			}
		}

		WHEN("XDG_CONFIG_HOME is set.")
		{
			const auto tempdir = temporary_directory();
			REQUIRE(setenv(xdg_home, tempdir.dirname.c_str(), true) == 0);
			const auto account_dir = account_directory_path_uncached();
			THEN("The path is as expected.")
			{
				const auto expectedpath = tempdir.dirname / "msync" / Account_Directory;
				REQUIRE(account_dir == expectedpath);
			}
		}

		if (oldhome == nullptr)
			unsetenv(xdg_home);
		else
			setenv(xdg_home, old_home_val.c_str(), true);
	}
	#endif
}
#endif
