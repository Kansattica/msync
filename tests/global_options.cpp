#include "test_helpers.hpp"
#include <catch2/catch.hpp>
#include <constants.hpp>

#include "../lib/options/global_options.hpp"

#include <string>
using namespace std::string_literals;

SCENARIO("Both paths we care about are directories.", "[global_options]")
{
    GIVEN("A global_options object")
    {
        global_options opts;

        WHEN("the object is created")
        {
            THEN("executable_location is actually a directory.")
            {
                REQUIRE(fs::is_directory(opts.executable_location));
            }
        }
    }
}

SCENARIO("add_new_account correctly handles input.", "[global_options]")
{
    GIVEN("A global_options object")
    {
        global_options opts;

        WHEN("add_new_account is called on it with a valid username")
        {
            const fs::path accountdir = opts.executable_location / Account_Directory;
            test_file fi{accountdir};

            fs::path userfile{accountdir};
            userfile /= "coolguy@website.com";
            userfile /= User_Options_Filename;

            auto& added = opts.add_new_account("coolguy@website.com");

            THEN("the accounts directory is created.")
            {
                REQUIRE(fs::exists(accountdir));
                REQUIRE(fs::is_directory(accountdir));
            }

            THEN("no file is created immediately.")
            {
                REQUIRE_FALSE(fs::exists(userfile));
            }

            AND_WHEN("the accounts are cleared")
            {
                opts.accounts.clear();

                THEN("a file is created at the correct place.")
                {
                    fs::path userfile{accountdir};
                    userfile /= "coolguy@website.com";
                    userfile /= User_Options_Filename;
                    REQUIRE(fs::exists(userfile));
                }
            }
        }
    }
}

SCENARIO("read_accounts correctly fills global_options on construction.", "[global_options]")
{
    GIVEN("A global_options with some values added to it, destroyed, and then a new one created.")
    {
        fs::path accountdir;
        {
            global_options opt;
            accountdir = opt.executable_location / Account_Directory;

            {
                test_file fi{accountdir};
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
            auto found = opt.accounts.find("coolaccount@website.com");

            THEN("something was found")
            {
                REQUIRE(found != opt.accounts.end());
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
            auto found = opt.accounts.find("boringaccount@badsize.pling");

            THEN("nothing was found")
            {
                REQUIRE(found == opt.accounts.end());
            }
        }
        test_file fi{accountdir};
    }
}

SCENARIO("select_account selects exactly one account.", "[global_options]")
{
    GIVEN("An empty accounts unordered_map")
    {
        global_options options;
        options.accounts.clear();

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
    }

    GIVEN("An accounts unordered_map with one entry")
    {
        const fs::path optiontestfile{"regulartest"};
        test_file fi{optiontestfile};

        global_options options;
        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});

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
    }

    GIVEN("An accounts unordered_map with two entries")
    {
        const fs::path optiontestfile{"regulartest"};
        test_file fi{optiontestfile};
        const fs::path anotheroptiontestfile{"regulartestguy"};
        test_file anotherfi{anotheroptiontestfile};

        global_options options;
        options.accounts.clear();
        options.accounts.insert({"someaccount@website.com", user_options(optiontestfile)});
        options.accounts.insert({"someotheraccount@place2.egg", user_options(anotheroptiontestfile)});

        for (auto& pair : options.accounts)
        {
            pair.second.set_option(user_option::account_name, pair.first);
        }

        WHEN("select_account is given an empty string to search on")
        {
            auto account = options.select_account("");

            THEN("a nullptr is returned")
            {
                REQUIRE(account == nullptr);
            }
        }

        WHEN("select_account is given a non-empty string to search on that's an ambiguous prefix")
        {
            auto account = options.select_account("some");

            THEN("a nullptr is returned")
            {
                REQUIRE(account == nullptr);
            }
        }

        WHEN("select_account is given a non-empty string to search on that's an unambiguous prefix")
        {
            auto account = options.select_account("someother");

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
            auto searchon = GENERATE("someotheR"s, "SomEOThEr", "SoMeOtHeR", "Someother");
            auto account = options.select_account(searchon);

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

        WHEN("select_account is given a non-empty string to search on that's not a valid prefix")
        {
            auto account = options.select_account("bad");

            THEN("a nullptr is returned")
            {
                REQUIRE(account == nullptr);
            }
        }

        WHEN("select_account is given a non-empty string to search on that's longer than the account names.")
        {
            auto account = options.select_account(std::string(100, 'b'));

            THEN("a nullptr is returned")
            {
                REQUIRE(account == nullptr);
            }
        }
    }
}