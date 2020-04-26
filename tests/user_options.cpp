#include "test_helpers.hpp"
#include <catch2/catch.hpp>

#include <fstream>

#include "../lib/options/user_options.hpp"

#include "../lib/exception/msync_exception.hpp"

SCENARIO("User_options reads from a file when created")
{
	GIVEN("A file on disk with some properly formatted data")
	{
		const test_file fi = temporary_file();

		{
			std::ofstream maketest(fi);

			maketest << "account_name=sometester\n";
			maketest << "instance_url=website.egg\n";
		}

		WHEN("a user_options is created from a filename")
		{
			const user_options opt(fi.filename);

			THEN("the fields are set correctly.")
			{
				REQUIRE(*opt.try_get_option(user_option::account_name) == "sometester");
				REQUIRE(*opt.try_get_option(user_option::instance_url) == "website.egg");
				REQUIRE(opt.get_option(user_option::account_name) == "sometester");
				REQUIRE(opt.get_option(user_option::instance_url) == "website.egg");
			}

			THEN("empty fields are handled gracefully.")
			{
				REQUIRE(opt.try_get_option(user_option::access_token) == nullptr);
				REQUIRE_THROWS_AS(opt.get_option(user_option::access_token), msync_exception);
			}

			THEN("No .bak file is created.")
			{
				REQUIRE_FALSE(fs::exists(fi.filenamebak));
			}
		}
	}
}

SCENARIO("User_options saves changes back to its file")
{
	GIVEN("A file on disk with some properly formatted data")
	{
		const test_file fi = temporary_file();

		{
			std::ofstream maketest(fi);

			maketest << "account_name=sometester\n";
			maketest << "instance_url=website.egg\n";
		}

		WHEN("a user_options is created from a filename and a field is changed")
		{
			{
				user_options opt(fi.filename);
				opt.set_option(user_option::account_name, "someoneelse");
			}

			THEN("a newly created user_options for the same path should see the change.")
			{
				const user_options newopt(fi.filename);
				REQUIRE(*newopt.try_get_option(user_option::account_name) == "someoneelse");
				REQUIRE(*newopt.try_get_option(user_option::instance_url) == "website.egg");
				REQUIRE(newopt.get_option(user_option::account_name) == "someoneelse");
				REQUIRE(newopt.get_option(user_option::instance_url) == "website.egg");
				REQUIRE(newopt.try_get_option(user_option::access_token) == nullptr);
				REQUIRE_THROWS_AS(newopt.get_option(user_option::access_token), msync_exception);
			}

			THEN("a .bak file with the original information should be created.")
			{
				REQUIRE(fs::exists(fi.filenamebak));

				const auto lines = read_lines(fi.filenamebak);

				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "account_name=sometester");
				REQUIRE(lines[1] == "instance_url=website.egg");
			}
		}
	}
}

SCENARIO("An empty user_options writes to a file when destroyed")
{
	GIVEN("A path with no file on disk")
	{
		const test_file fi = temporary_file();

		WHEN("a user_options is created with that path and modified")
		{
			{
				user_options opt(fi.filename);

				opt.set_option(user_option::account_name, "somejerk");
				opt.set_option(user_option::instance_url, "rude.website");
				opt.set_option(user_option::pull_dms, sync_settings::newest_first);

				THEN("the file is not created until the user_options is destroyed")
				{
					REQUIRE_FALSE(fs::exists(fi.filename));
				}
			}

			THEN("The file is created when user_options is destroyed.")
			{
				REQUIRE(fs::exists(fi.filename));

				const auto lines = read_lines(fi.filename);

				REQUIRE(lines.size() == 4);
				REQUIRE(lines[0] == "account_name=somejerk");
				REQUIRE(lines[1] == "file_version=1");
				REQUIRE(lines[2] == "instance_url=rude.website");
				REQUIRE(lines[3] == "pull_dms=newest_first");
			}

			THEN("No .bak file is created.")
			{
				REQUIRE_FALSE(fs::exists(fi.filenamebak));
			}
		}
	}
}

SCENARIO("Get and set options manage string references correctly.")
{
	GIVEN("A file on disk with some properly formatted data")
	{
		const test_file fi = temporary_file();

		{
			std::ofstream maketest(fi);

			maketest << "account_name=sometester\n";
			maketest << "instance_url=website.egg\n";
		}

		WHEN("an option that exists in the dictionary is requested")
		{
			user_options opt(fi.filename);
			auto val = opt.try_get_option(user_option::account_name);
			auto& refval = opt.get_option(user_option::account_name);

			THEN("the value is present.")
			{
				REQUIRE(val != nullptr);
			}

			THEN("the value is correct.")
			{
				REQUIRE(*val == "sometester");
				REQUIRE(*val == refval);
			}

			AND_WHEN("the option is modified")
			{
				opt.set_option(user_option::account_name, "someotherguy");

				THEN("the change is reflected in the original value")
				{
					REQUIRE(*val == "someotherguy");
					REQUIRE(refval == "someotherguy");
				}
			}
		}
	}
}

CATCH_REGISTER_ENUM(sync_settings, sync_settings::dont_sync, sync_settings::newest_first, sync_settings::oldest_first)

SCENARIO("The enum overload for get_option works.")
{
	const test_file fi = temporary_file();
	GIVEN("An empty user_options")
	{
		const user_options opt{fi.filename};

		WHEN("one of the three options that have sync settings is asked for.")
		{
			auto option = GENERATE(user_option::pull_home, user_option::pull_dms, user_option::pull_notifications);
			auto result = opt.get_sync_option(option);

			THEN("the result has the correct default.")
			{
				//newest_first for home, oldest_first for the other two
				if (option == user_option::pull_home)
				{
					REQUIRE(result == sync_settings::newest_first);
				}
				else
				{
					REQUIRE(result == sync_settings::oldest_first);
				}
			}
		}
	}

	GIVEN("A user_options with some of the pull options set.")
	{
		user_options opt{fi.filename};
		opt.set_option(user_option::pull_home, sync_settings::oldest_first);

		WHEN("one of the three options that have sync settings is asked for.")
		{
			const auto option = GENERATE(user_option::pull_home, user_option::pull_dms, user_option::pull_notifications);
			const auto result = opt.get_sync_option(option);

			THEN("the result has the correct set or default value.")
			{
				REQUIRE(result == sync_settings::oldest_first);
			}
		}

		WHEN("the user_options is destroyed")
		{
			{
				const user_options newopt = std::move(opt);
			}

			THEN("The generated file has the correct option set.")
			{
				const auto lines = read_lines(fi.filename);

				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "file_version=1");
				REQUIRE(lines[1] == "pull_home=oldest_first");
			}

			AND_WHEN("a new user_options is created from that file")
			{
				const user_options neweropt(fi.filename);

				THEN("it has the correct value.")
				{
					REQUIRE(neweropt.get_sync_option(user_option::pull_home) == sync_settings::oldest_first);
					REQUIRE(*neweropt.try_get_option(user_option::pull_home) == "oldest_first");
					REQUIRE(neweropt.get_option(user_option::pull_home) == "oldest_first");
				}
			}
		}
	}
}

SCENARIO("The boolean overload for get_option works.")
{
	const test_file fi = temporary_file();
	GIVEN("An empty user_options")
	{
		user_options opt{fi.filename};

		WHEN("one of the five boolean options is asked for.")
		{
			const auto option = GENERATE(user_option::exclude_boosts, user_option::exclude_favs,
				user_option::exclude_follows, user_option::exclude_mentions, user_option::exclude_polls);

			const auto result = opt.get_bool_option(option);

			THEN("the result has the correct default.")
			{
				REQUIRE(result == false);
			}
		}
	}

	GIVEN("An empty user_options")
	{
		user_options opt{fi.filename};

		WHEN("one of the five boolean options is set to 'true' or 'yes'.")
		{
			const auto option = GENERATE(user_option::exclude_boosts, user_option::exclude_favs,
				user_option::exclude_follows, user_option::exclude_mentions, user_option::exclude_polls);

			const auto value = GENERATE(as<std::string>{}, "true", "True", "TRUE", "yes", "Yes", "YES", "Yeehaw", "yeehaw", "t", "T", "y", "Y");

			opt.set_option(option, value);

			THEN("the result has been set to true.")
			{
				const auto result = opt.get_bool_option(option);
				REQUIRE(result == true);
			}
		}

		WHEN("one of the five boolean options is set to anything else")
		{
			const auto option = GENERATE(user_option::exclude_boosts, user_option::exclude_favs,
				user_option::exclude_follows, user_option::exclude_mentions, user_option::exclude_polls);

			const auto value = GENERATE(as<std::string>{}, "false", "no", "knock it off", "", ":(", "No");

			opt.set_option(option, value);

			THEN("the result has been set correctly.")
			{
				const auto result = opt.get_bool_option(option);
				REQUIRE(result == false);
			}
		}
	}

	GIVEN("A user_options with some of the exclude options set.")
	{
		user_options opt{fi.filename};
		opt.set_bool_option(user_option::exclude_boosts, true);

		WHEN("one of the options that have boolean settings is asked for.")
		{
			const auto option = GENERATE(user_option::exclude_boosts, user_option::exclude_favs,
				user_option::exclude_follows, user_option::exclude_mentions, user_option::exclude_polls);

			const auto result = opt.get_bool_option(option);

			THEN("the result has the correct set or default value.")
			{
				if (option == user_option::exclude_boosts)
				{
					REQUIRE(result == true);
				}
				else
				{
					REQUIRE(result == false);
				}
			}
		}

		WHEN("the user_options is destroyed")
		{
			{
				user_options newopt = std::move(opt);
			}

			THEN("The generated file has the correct option set.")
			{
				auto lines = read_lines(fi.filename);

				REQUIRE(lines.size() == 2);
				REQUIRE(lines[0] == "exclude_boosts=true");
				REQUIRE(lines[1] == "file_version=1");
			}

			AND_WHEN("a new user_options is created from that file")
			{
				user_options neweropt(fi.filename);

				THEN("it has the correct value.")
				{
					REQUIRE(neweropt.get_bool_option(user_option::exclude_boosts) == true);
					REQUIRE(*neweropt.try_get_option(user_option::exclude_boosts) == "true");
					REQUIRE(neweropt.get_option(user_option::exclude_boosts) == "true");
				}
			}
		}
	}
}

SCENARIO("user_options correctly reports the directory it's in.")
{
	GIVEN("A user_options for some file is created and destroyed.")
	{
		const test_file fi = temporary_file();

		{ // gotta make sure that file gets made
			user_options opts{ fi.filename };
		}

		WHEN("get_user_directory is taken from a new user_options.")
		{
			user_options opts{ fi.filename };
			const auto& userdir = opts.get_user_directory();

			THEN("The directory exists.")
			{
				REQUIRE(fs::is_directory(userdir));
			}

			THEN("The directory contains the user_options.")
			{
				REQUIRE(fs::is_regular_file(userdir / fi.filename.filename()));
			}
		}
	}
}