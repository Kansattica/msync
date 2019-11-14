#include "test_helpers.hpp"
#include <catch2/catch.hpp>
#include <constants.hpp>

#include "../lib/options/global_options.hpp"

#include <string>
using namespace std::string_literals;

SCENARIO("add_new_account correctly handles input.")
{
	const test_file acc = account_directory();
	GIVEN("A global_options object")
	{
		global_options opts;

		WHEN("add_new_account is called on it with a valid username")
		{
			fs::path userfile{ acc.filename };
			userfile /= "coolguy@website.com";
			userfile /= User_Options_Filename;

			const auto& added = opts.add_new_account("coolguy@website.com");

			THEN("the accounts directory is created.")
			{
				REQUIRE(fs::exists(acc.filename));
				REQUIRE(fs::is_directory(acc.filename));
			}

			THEN("no file is created immediately.")
			{
				REQUIRE_FALSE(fs::exists(userfile));
			}

			THEN("it shows up in all_accounts.")
			{
				const auto accounts = opts.all_accounts();
				REQUIRE(accounts.size() == 1);
				REQUIRE(accounts[0] == "coolguy@website.com");
			}

			AND_WHEN("the options are moved from and destroyed.")
			{
				{
					global_options temp = std::move(opts);
				}

				THEN("a file is created at the correct place.")
				{
					REQUIRE(fs::exists(userfile));
				}
			}

			AND_WHEN("the same account is added again")
			{
				const auto& alsoadded = opts.add_new_account("coolguy@website.com");

				THEN("the objects are the same")
				{
					REQUIRE(&alsoadded == &added);
				}

				THEN("there's still only one account in all_accounts")
				{
					const auto accounts = opts.all_accounts();
					REQUIRE(accounts.size() == 1);
					REQUIRE(accounts[0] == "coolguy@website.com");
				}
			}
		}
	}
}

SCENARIO("read_accounts correctly fills global_options on construction.")
{
	const test_file acc = account_directory();
	GIVEN("A global_options with some values added to it, destroyed, and then a new one created.")
	{
		{
			global_options opt;

			{
				test_file fi{ acc.filename };
			}

			auto& added = opt.add_new_account("coolaccount@website.com");
			REQUIRE(added.first == "coolaccount@website.com");
			added.second.set_option(user_option::account_name, "coolaccount");
			added.second.set_option(user_option::instance_url, "website.com");
			auto& alsoadded = opt.add_new_account("evencooleraccount@wedsize.egg");
			REQUIRE(alsoadded.first == "evencooleraccount@wedsize.egg");
			alsoadded.second.set_option(user_option::account_name, "evencooleraccount");
			alsoadded.second.set_option(user_option::instance_url, "wedsize.egg");
		}

		global_options opt;

		WHEN("a given account is looked up")
		{
			const auto found = opt.select_account("coolaccount@website.com");

			THEN("something was found")
			{
				REQUIRE(found != nullptr);
			}

			THEN("it was what we expected")
			{
				REQUIRE(found->first == "coolaccount@website.com");
				REQUIRE(*found->second.get_option(user_option::account_name) == "coolaccount");
				REQUIRE(*found->second.get_option(user_option::instance_url) == "website.com");
			}
		}

		WHEN("an incorrect account is looked up")
		{
			const auto found = opt.select_account("boringaccount@badsize.pling");

			THEN("nothing was found")
			{
				REQUIRE(found == nullptr);
			}
		}

		WHEN("all_accounts is called")
		{
			const auto accounts = opt.all_accounts();

			THEN("both accounts show up.")
			{
				using Catch::Matchers::UnorderedEquals;
				REQUIRE_THAT(accounts, Catch::UnorderedEquals(std::vector<std::string_view> {"coolaccount@website.com", "evencooleraccount@wedsize.egg"}));
			}
		}
	}
}

SCENARIO("select_account selects exactly one account.")
{
	// this should make sure the accounts are empty
	const test_file acc = account_directory();
	GIVEN("An empty accounts unordered_map")
	{
		global_options options;

		WHEN("select_account is given an empty string to search on")
		{
			auto account = options.select_account("");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on")
		{
			auto account = options.select_account("coolperson");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("all_accounts is called")
		{
			const auto accounts = options.all_accounts();

			THEN("no accounts show up.")
			{
				REQUIRE(accounts.empty());
			}
		}
	}

	GIVEN("An accounts unordered_map with one entry")
	{
		const test_file fi = acc.filename / "someaccount@website.com";

		global_options options;
		options.add_new_account("someaccount@website.com");

		WHEN("select_account is given an empty string to search on")
		{
			auto account = options.select_account("");

			THEN("a user_options is returned.")
			{
				REQUIRE_FALSE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix")
		{
			auto account = options.select_account("some");

			THEN("a user_options is returned.")
			{
				REQUIRE_FALSE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix, but capitalized differently")
		{
			auto searchon = GENERATE("Some"s, "sOme", "SOme", "soME", "SOME", "somE");
			auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE_FALSE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
		{
			auto account = options.select_account("bad");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("all_accounts is called")
		{
			const auto accounts = options.all_accounts();

			THEN("the account shows up.")
			{
				REQUIRE(accounts.size() == 1);
				REQUIRE(accounts[0] == "someaccount@website.com");
			}
		}
	}

	GIVEN("An accounts unordered_map with two entries")
	{
		const test_file fi = acc.filename / "someaccount@website.com";
		const test_file anotherfi = acc.filename / "someotheraccount@place2.egg";

		global_options options;

		for (const auto& accountname : { "someaccount@website.com", "someotheraccount@place2.egg" })
		{
			auto& newaccount = options.add_new_account(accountname);
			newaccount.second.set_option(user_option::account_name, accountname);
		}

		WHEN("select_account is given an empty string to search on")
		{
			const auto account = options.select_account("");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an ambiguous prefix")
		{
			const auto account = options.select_account("some");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix")
		{
			const auto account = options.select_account("someother");

			THEN("a user_options is returned.")
			{
				REQUIRE_FALSE(account == nullptr);
			}

			THEN("the user_options is the correct one.")
			{
				const std::string account_name = *account->second.get_option(user_option::account_name);
				REQUIRE(account_name == "someotheraccount@place2.egg");
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix, but capitalized differently")
		{
			const auto searchon = GENERATE("someotheR"s, "SomEOThEr", "SoMeOtHeR", "Someother");
			const auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE_FALSE(account == nullptr);
			}

			THEN("the user_options is the correct one.")
			{
				REQUIRE(*account->second.get_option(user_option::account_name) == "someotheraccount@place2.egg");
			}
		}

		WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
		{
			const auto account = options.select_account("bad");

			THEN("a nullptr is returned")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's longer than the account names")
		{
			const auto account = options.select_account(std::string(100, 'b'));

			THEN("a nullptr is returned.")
			{
				REQUIRE(account == nullptr);
			}
		}

		WHEN("each user account is visited by foreach_account")
		{
			int visited = 0;
			options.foreach_account([&](const auto& pair) { visited++; });

			THEN("each account is visited once.")
			{
				REQUIRE(visited == 2);
			}
		}

		WHEN("all_accounts is called")
		{
			const auto accounts = options.all_accounts();

			THEN("both accounts show up.")
			{
				using Catch::Matchers::UnorderedEquals;
				REQUIRE_THAT(accounts, Catch::UnorderedEquals(std::vector<std::string_view> {"someaccount@website.com", "someotheraccount@place2.egg" }));
			}
		}
	}
}