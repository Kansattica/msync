#include "test_helpers.hpp"
#include <catch2/catch.hpp>
#include <constants.hpp>
#include <print_logger.hpp>

#include "../lib/options/global_options.hpp"

#include <string>
#include <array>
#include <string_view>
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
#ifndef _WIN32
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
				REQUIRE(std::holds_alternative<user_ptr>(result));
			}

			THEN("it was what we expected")
			{
				const auto found = std::get<user_ptr>(result);
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
				REQUIRE(std::holds_alternative<user_ptr>(result));
			}

			THEN("it was what we expected")
			{
				const auto found = std::get<user_ptr>(result);
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
				REQUIRE(std::holds_alternative<select_account_error>(found));
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
				REQUIRE(std::holds_alternative<select_account_error>(account));
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
				REQUIRE(std::holds_alternative<select_account_error>(account));
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
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<user_ptr>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix")
		{
			auto account = options.select_account("some");

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<user_ptr>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix, but capitalized differently")
		{
			auto searchon = GENERATE("Some"sv, "sOme", "SOme", "soME", "SOME", "somE");
			auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<user_ptr>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's a valid prefix, including an @, but capitalized differently")
		{
			auto searchon = GENERATE("@Some"sv, "@sOme", "@SOme", "@soME", "@SOME", "@somE");
			auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options has the correct name.")
			{
				REQUIRE(std::get<user_ptr>(account)->first == expected_account);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
		{
			auto account = options.select_account("bad");

			THEN("an error is returned.")
			{
				REQUIRE(std::holds_alternative<select_account_error>(account));
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
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<user_ptr>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix with a leading @")
		{
			const auto account = options.select_account("@someother");

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<user_ptr>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix, but capitalized differently")
		{
			const auto searchon = GENERATE("someotheR"sv, "SomEOThEr", "SoMeOtHeR", "Someother");
			const auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<user_ptr>(account)->second.get_option(user_option::account_name);
				REQUIRE(account_name == expected_accounts[1]);
			}
		}

		WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix with an @, but capitalized differently")
		{
			const auto searchon = GENERATE("@someotheR"sv, "@SomEOThEr", "@SoMeOtHeR", "@Someother");
			const auto account = options.select_account(searchon);

			THEN("a user_options is returned.")
			{
				REQUIRE(std::holds_alternative<user_ptr>(account));
			}

			THEN("the user_options is the correct one.")
			{
				const std::string& account_name = std::get<user_ptr>(account)->second.get_option(user_option::account_name);
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
			std::vector<std::string_view> visited_accounts;
			options.foreach_account([&](const auto& user) { visited_accounts.push_back(user.first); });

			THEN("each account is visited once.")
			{
				REQUIRE(visited_accounts.size() == 2);
				REQUIRE_THAT(std::vector<std::string_view>(expected_accounts.begin(), expected_accounts.end()),
					Catch::UnorderedEquals(visited_accounts));
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

SCENARIO("select_account respects the is_default setting.")
{
	logs_off = true;
	GIVEN("A global_options with several accounts, only one of which is the default.")
	{
		const test_dir acc = temporary_directory();

		constexpr std::array<std::string_view, 4> accounts =
			{ "somebody@crime.egg", "someoneelse@crime.egg", "zimbo@illegal.egg", "zoobin@illegal.egg" };

		const auto expected_default = GENERATE_COPY(from_range(accounts));

		global_options opts{ acc.dirname };
		for (const auto& acct : accounts)
		{
			opts.add_new_account(std::string{ acct });
		} 

		WHEN("The default is set with an ambiguous prefix.")
		{
			const auto prefix = GENERATE("some", "Some", "SOME", "sOmE", "z", "Z");
			const auto result = opts.set_default(prefix);

			THEN("The correct error is returned.")
			{
				REQUIRE(std::holds_alternative<select_account_error>(result));
				REQUIRE(std::get<select_account_error>(result) == select_account_error::ambiguous_prefix);
			}

			THEN("Nothing is set as the default.")
			{
				const auto selected = opts.select_account({});

				REQUIRE(std::holds_alternative<select_account_error>(selected));
				REQUIRE(std::get<select_account_error>(selected) == select_account_error::empty_name_many_accounts);
			}
		}

		WHEN("The default is set with a correct, unambiguous account string.")
		{
			const auto prefix_length = GENERATE(0, 5, 10);
			auto prefix_to_search = expected_default;
			if (prefix_length != 0)
				prefix_to_search.remove_suffix(prefix_to_search.size() - prefix_length);

			REQUIRE(std::get<user_ptr>(opts.set_default(prefix_to_search))->first == expected_default);

			AND_WHEN("An empty string is given to select_account.")
			{
				const auto selected = opts.select_account({});

				THEN("The selected account is the default.")
				{
					REQUIRE(std::holds_alternative<user_ptr>(selected));
					REQUIRE(std::get<user_ptr>(selected)->first == expected_default);
					REQUIRE(std::get<user_ptr>(selected)->second.get_bool_option(user_option::is_default));
				}
			}

			AND_WHEN("An account name is given to select_account.")
			{
				THEN("The correct account is selected.")
				{
					for (const auto& account : accounts)
					{
						const auto selected = opts.select_account(account);
						REQUIRE(std::holds_alternative<user_ptr>(selected));
						REQUIRE(std::get<user_ptr>(selected)->first == account);
					}
				}
			}

			AND_WHEN("An unambiguous prefix is given to select_account.")
			{
				const auto selected = opts.select_account("zimbo");

				THEN("The selected account is the correct one.")
				{
					REQUIRE(std::holds_alternative<user_ptr>(selected));
					REQUIRE(std::get<user_ptr>(selected)->first == "zimbo@illegal.egg");
				}
			}

			AND_WHEN("An ambiguous prefix is given to select_account.")
			{
				const auto selected = opts.select_account("SOME");

				THEN("The selected account is the correct one.")
				{
					REQUIRE(std::holds_alternative<select_account_error>(selected));
					REQUIRE(std::get<select_account_error>(selected) == select_account_error::ambiguous_prefix);
				}
			}

			AND_WHEN("A prefix that doesn't match anything is given to select_account.")
			{
				const auto selected = opts.select_account("asdfasdf");

				THEN("The selected account is the correct one.")
				{
					REQUIRE(std::holds_alternative<select_account_error>(selected));
					REQUIRE(std::get<select_account_error>(selected) == select_account_error::bad_prefix);
				}
			}

			AND_WHEN("The default is updated.")
			{
				const auto expected_new_default = GENERATE_COPY(from_range(accounts));

				REQUIRE(std::get<user_ptr>(opts.set_default(expected_new_default))->first == expected_new_default);

				AND_WHEN("An empty string is given to select_account.")
				{
					const auto selected = opts.select_account({});

					THEN("The selected account is the default.")
					{
						REQUIRE(std::holds_alternative<user_ptr>(selected));
						REQUIRE(std::get<user_ptr>(selected)->first == expected_new_default);
						REQUIRE(std::get<user_ptr>(selected)->second.get_bool_option(user_option::is_default));
					}

					THEN("The old default is no longer marked as the default.")
					{
						if (expected_default != expected_new_default)
						{
							const auto old_default = opts.select_account(expected_default);
							REQUIRE(std::holds_alternative<user_ptr>(old_default));
							REQUIRE(std::get<user_ptr>(old_default)->first == expected_default);
							REQUIRE_FALSE(std::get<user_ptr>(old_default)->second.get_bool_option(user_option::is_default));
						}
					}
				}
			}

			AND_WHEN("The default is cleared.")
			{
				REQUIRE(std::get<user_ptr>(opts.set_default({})) == nullptr);

				THEN("Passing an empty string to select_account returns an error.")
				{
					const auto selected = opts.select_account({});

					REQUIRE(std::holds_alternative<select_account_error>(selected));
					REQUIRE(std::get<select_account_error>(selected) == select_account_error::empty_name_many_accounts);
				}
			}

			AND_WHEN("The default is set again with an ambiguous prefix.")
			{
				const auto prefix = GENERATE("some", "Some", "SOME", "sOmE", "z", "Z");
				const auto result = opts.set_default(prefix);

				THEN("The correct error is returned.")
				{
					REQUIRE(std::holds_alternative<select_account_error>(result));
					REQUIRE(std::get<select_account_error>(result) == select_account_error::ambiguous_prefix);
				}

				THEN("The default is unchanged.")
				{
					const auto selected = opts.select_account({});

					REQUIRE(std::holds_alternative<user_ptr>(selected));
					REQUIRE(std::get<user_ptr>(selected)->first == expected_default);
					REQUIRE(std::get<user_ptr>(selected)->second.get_bool_option(user_option::is_default));
				}
			}
		}
	}
}
