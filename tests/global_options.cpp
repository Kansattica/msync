#include "test_helpers.hpp"
#include <catch2/catch.hpp>
#include <constants.hpp>
#include <print_logger.hpp>

#include "../lib/options/global_options.hpp"

#include <string>
using namespace std::string_view_literals;

SCENARIO("add_new_account correctly handles input.")
{
	logs_off = true;
	const test_dir acc = temporary_directory();
	GIVEN("A global_options object")
	{
		global_options opts{ acc.dirname };

		WHEN("add_new_account is called on it with a valid username")
		{
			fs::path userfile{ acc.dirname };
			userfile /= "coolguy@website.com";
			userfile /= User_Options_Filename;

			const auto& added = opts.add_new_account("coolguy@website.com");

			THEN("the accounts directory is created.")
			{
				REQUIRE(fs::exists(acc.dirname));
				REQUIRE(fs::is_directory(acc.dirname));
			}

			//Windows doesn't respect filesystem permissions in the same way.
#ifdef __linux__
			THEN("the accounts directory has the proper permissions.")
			{
				REQUIRE(fs::status(acc.dirname).permissions() == fs::perms::owner_all);
			}
#endif

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
	const test_dir acc = temporary_directory();
	GIVEN("A global_options with some values added to it, destroyed, and then a new one created.")
	{
		{
			global_options opt{ acc.dirname };

			auto& added = opt.add_new_account("coolaccount@website.com");
			REQUIRE(added.first == "coolaccount@website.com");
			added.second.set_option(user_option::account_name, "coolaccount");
			added.second.set_option(user_option::instance_url, "website.com");
			auto& alsoadded = opt.add_new_account("evencooleraccount@wedsize.egg");
			REQUIRE(alsoadded.first == "evencooleraccount@wedsize.egg");
			alsoadded.second.set_option(user_option::account_name, "evencooleraccount");
			alsoadded.second.set_option(user_option::instance_url, "wedsize.egg");
		}

		global_options opt{ acc.dirname };

		WHEN("a given account is looked up")
		{
			const auto result = opt.select_account("coolaccount@website.com");

			THEN("something was found")
			{
				REQUIRE(result.index() == 0);
			}

			THEN("it was what we expected")
			{
				const auto found = std::get<0>(result);
				REQUIRE(found->first == "coolaccount@website.com");
				REQUIRE(found->second.get_option(user_option::account_name) == "coolaccount");
				REQUIRE(found->second.get_option(user_option::instance_url) == "website.com");
			}
		}

		WHEN("a given account is looked up with a leading @")
		{
			const auto result = opt.select_account("@coolaccount@website.com");

			THEN("something was found")
			{
				REQUIRE(result.index() == 0);
			}

			THEN("it was what we expected")
			{
				const auto found = std::get<0>(result);
				REQUIRE(found->first == "coolaccount@website.com");
				REQUIRE(found->second.get_option(user_option::account_name) == "coolaccount");
				REQUIRE(found->second.get_option(user_option::instance_url) == "website.com");
			}
		}

		WHEN("an incorrect account is looked up")
		{
			const auto found = opt.select_account("boringaccount@badsize.pling");

			THEN("an error is returned.")
			{
				REQUIRE(found.index() == 1);
			}

			THEN("the error correctly reports that there were no matches because the prefix didn't match.")
			{
				REQUIRE(std::get<select_account_error>(found) == select_account_error::bad_prefix);
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
	const test_dir acc = temporary_directory();
	GIVEN("An empty accounts unordered_map")
	{
		global_options options{ acc.dirname };

		WHEN("select_account is given an empty string to search on")
		{
			REQUIRE_NOTHROW(options.select_account(""));
			auto account = options.select_account("");

			THEN("a error is returned")
			{
				REQUIRE(account.index() == 1);
			}

			THEN("the error correctly reports that there are no accounts.")
			{
				REQUIRE(std::get<select_account_error>(account) == select_account_error::no_accounts);
			}
		}

		WHEN("select_account is given a non-empty string to search on")
		{
			auto account = options.select_account("coolperson");

			THEN("a error is returned")
			{
				REQUIRE(account.index() == 1);
			}

			THEN("the error correctly reports that there are no accounts.")
			{
				REQUIRE(std::get<select_account_error>(account) == select_account_error::no_accounts);
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

	GIVEN("A global_options with one account")
	{
		const static std::string expected_account = "someaccount@website.com";
		const test_file fi = acc.dirname / expected_account;

		global_options options{ acc.dirname };
		options.add_new_account(expected_account);

		WHEN("select_account is given an empty string to search on")
		{
			REQUIRE_NOTHROW(options.select_account(""));
			auto account = options.select_account("");

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<0>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix")
		{
			auto account = options.select_account("some");

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<0>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix, but capitalized differently")
		{
			auto searchon = GENERATE("Some"sv, "sOme", "SOme", "soME", "SOME", "somE");
			auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<0>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix, including an @, but capitalized differently")
		{
			auto searchon = GENERATE("@Some"sv, "@sOme", "@SOme", "@soME", "@SOME", "@somE");
			auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<0>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
		{
			auto account = options.select_account("bad");

			THEN("an error is returned.")
			{
				REQUIRE(account.index() == 1);
			}

			THEN("the error correctly reports that an invalid prefix was given.")
			{
				REQUIRE(std::get<select_account_error>(account) == select_account_error::bad_prefix);
			}
		}

		WHEN("all_accounts is called")
		{
			const auto accounts = options.all_accounts();

			THEN("the account shows up.")
			{
				REQUIRE(accounts.size() == 1);
				REQUIRE(accounts[0] == expected_account);
			}
		}
	}

	GIVEN("An global_options with two accounts")
	{
		const static std::array<std::string, 2> expected_accounts { "someaccount@website.com",  "someotheraccount@place2.egg" };
		const test_file fi = acc.dirname / expected_accounts[0];
		const test_file anotherfi = acc.dirname / expected_accounts[1];

		global_options options{ acc.dirname };

		for (const auto& accountname : expected_accounts)
		{
			auto& newaccount = options.add_new_account(accountname);
			newaccount.second.set_option(user_option::account_name, accountname);
		}

		WHEN("select_account is given an empty string to search on")
		{
			const auto account = options.select_account("");

			THEN("an error is returned to indicate that an empty string was given when there's more than one account.")
			{
				REQUIRE(std::get<select_account_error>(account) == select_account_error::empty_name_many_accounts);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an ambiguous prefix")
		{
			const auto account = options.select_account("some");

			THEN("an error is returned to indicate that an ambiguous prefix was given.")
			{
				REQUIRE(std::get<select_account_error>(account) == select_account_error::ambiguous_prefix);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix")
		{
			const auto account = options.select_account("someother");

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<0>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix with a leading @")
		{
			const auto account = options.select_account("@someother");

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<0>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix, but capitalized differently")
		{
			const auto searchon = GENERATE("someotheR"sv, "SomEOThEr", "SoMeOtHeR", "Someother");
			const auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<0>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix with an @, but capitalized differently")
		{
			const auto searchon = GENERATE("@someotheR"sv, "@SomEOThEr", "@SoMeOtHeR", "@Someother");
			const auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(account.index() == 0);
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<0>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
		{
			const auto account = options.select_account("bad");

			THEN("an error is returned that indicates that an invalid prefix was requested.")
			{
				REQUIRE(std::get<1>(account) == select_account_error::bad_prefix);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's longer than the account names")
		{
			const auto account = options.select_account(std::string(100, 'b'));

			THEN("an error is returned that indicates that an invalid prefix was requested.")
			{
				REQUIRE(std::get<1>(account) == select_account_error::bad_prefix);
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
				REQUIRE_THAT(accounts, Catch::UnorderedEquals(std::vector<std::string_view>{expected_accounts.begin(), expected_accounts.end()}));
			}
		}
	}
}
