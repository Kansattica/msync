#include <catch2/catch.hpp>

#include <vector>
#include <random>
#include <algorithm>
#include <utility>

#include "test_helpers.hpp"

#include "../console/optionparsing/parse_options.hpp"

SCENARIO("The command line parser recognizes when the user wants to start a new account.")
{
	GIVEN("A command line that doesn't specify an account.")
	{
		constexpr int argc = 2;
		char const* argv[]{ "msync", "new" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is new user")
			{
				REQUIRE(parsed.selected == mode::newuser);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that does specify an account.")
	{
		constexpr int argc = 4;
		char const* argv[]{ "msync", "new", "-a", "regular@folks.egg" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is new user")
			{
				REQUIRE(parsed.selected == mode::newuser);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "regular@folks.egg");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}
}

SCENARIO("The command line parser extracts configuration option lines correctly.")
{
	GIVEN("A command line setting an option without an account.")
	{
		constexpr int argc = 4;
		char const* argv[]{ "msync", "config", "access_token", "sometoken" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is config")
			{
				REQUIRE(parsed.selected == mode::config);
			}

			THEN("the correct option will be changed")
			{
				REQUIRE(parsed.toset == user_option::access_token);
			}

			THEN("the option is correctly set")
			{
				REQUIRE(parsed.optionval == "sometoken");
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line setting an option with an account.")
	{
		constexpr int argc = 6;
		char const* argv[]{ "msync", "config", "client_secret", "asecret!", "-a", "jerk@fun.website" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is config")
			{
				REQUIRE(parsed.selected == mode::config);
			}

			THEN("the correct option will be changed")
			{
				REQUIRE(parsed.toset == user_option::client_secret);
			}

			THEN("the option is correctly set")
			{
				REQUIRE(parsed.optionval == "asecret!");
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "jerk@fun.website");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line reading an option without an account.")
	{
		constexpr int argc = 3;
		char const* argv[]{ "msync", "config", "account_name" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is showopt")
			{
				REQUIRE(parsed.selected == mode::showopt);
			}

			THEN("the correct option will be read")
			{
				REQUIRE(parsed.toset == user_option::account_name);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line reading an option with an account.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "instance_url", "-a", "niceperson@impolite.egg" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is showopt")
			{
				REQUIRE(parsed.selected == mode::showopt);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the correct option will be read")
			{
				REQUIRE(parsed.toset == user_option::instance_url);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "niceperson@impolite.egg");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A showall command that doesn't specify an account.")
	{
		constexpr int argc = 3;
		char const* argv[]{ "msync", "config", "showall" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is showallopt")
			{
				REQUIRE(parsed.selected == mode::showallopt);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A showall command that specifies an account.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "showall", "-a", "regular@folks.egg" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is showallopt")
			{
				REQUIRE(parsed.selected == mode::showallopt);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "regular@folks.egg");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A default command that doesn't specify an account.")
	{
		constexpr int argc = 3;
		char const* argv[]{ "msync", "config", "default" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is defaultopt")
			{
				REQUIRE(parsed.selected == mode::setdefault);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A default command that specifies an account.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "default", "-a", "regular@folks.egg" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is defaultopt")
			{
				REQUIRE(parsed.selected == mode::setdefault);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "regular@folks.egg");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	// lists aren't implemented yet, so don't bother
	/*GIVEN("A command line adding a list to be pulled.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "list", "add", "somelist" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configlist")
			{
				REQUIRE(parsed.selected == mode::configlist);
			}

			THEN("the option is set")
			{
				REQUIRE(parsed.optionval == "somelist");
			}

			THEN("the correct list operation is set")
			{
				REQUIRE(parsed.listops == list_operations::add);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line specifying a list to be removed.")
	{
		constexpr int argc = 7;
		char const* argv[]{ "msync", "config", "list", "remove", "anotherlist", "-a", "coolfriend" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configlist")
			{
				REQUIRE(parsed.selected == mode::configlist);
			}

			THEN("the option is set")
			{
				REQUIRE(parsed.optionval == "anotherlist");
			}

			THEN("the correct list operation is set")
			{
				REQUIRE(parsed.listops == list_operations::remove);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "coolfriend");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}*/

	GIVEN("A command line specifying that the home timeline should be synced oldest first.")
	{
		constexpr int argc = 7;
		char const* argv[]{ "msync", "config", "sync", "home", "oldest", "-a", "coolerfriend" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configsync")
			{
				REQUIRE(parsed.selected == mode::configsync);
			}

			THEN("the correct sync location is set")
			{
				REQUIRE(parsed.toset == user_option::pull_home);
			}

			THEN("the correct sync operation is set")
			{
				REQUIRE(parsed.sync_opts.mode == sync_settings::oldest_first);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "coolerfriend");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	/*GIVEN("A command line specifying that DMs should be synced newest first.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "sync", "dms", "newest" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configsync")
			{
				REQUIRE(parsed.selected == mode::configsync);
			}

			THEN("the correct sync location is set")
			{
				REQUIRE(parsed.toset == user_option::pull_dms);
			}

			THEN("the correct sync operation is set")
			{
				REQUIRE(parsed.sync_opts.mode == sync_settings::newest_first);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}*/

	GIVEN("A command line specifying that the notifications timeline should not be synced.")
	{
		constexpr int argc = 7;
		char const* argv[]{ "msync", "config", "sync", "notifications", "off", "-a", "coolestfriend" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configsync")
			{
				REQUIRE(parsed.selected == mode::configsync);
			}

			THEN("the correct sync location is set")
			{
				REQUIRE(parsed.toset == user_option::pull_notifications);
			}

			THEN("the correct sync operation is set")
			{
				REQUIRE(parsed.sync_opts.mode == sync_settings::dont_sync);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "coolestfriend");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line specifying that bookmarks should be synced newest first.")
	{
		constexpr int argc = 5;
		char const* argv[]{ "msync", "config", "sync", "bookmarks", "newest" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is configsync")
			{
				REQUIRE(parsed.selected == mode::configsync);
			}

			THEN("the correct sync location is set")
			{
				REQUIRE(parsed.toset == user_option::pull_bookmarks);
			}

			THEN("the correct sync operation is set")
			{
				REQUIRE(parsed.sync_opts.mode == sync_settings::newest_first);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}
}

SCENARIO("The command line parser recognizes when the user wants to sync.")
{

	char const* subcommand = GENERATE("sync", "s");
	GIVEN("A command line that says 'sync'.")
	{
		constexpr int argc = 2;
		char const* argv[]{ "msync", subcommand };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("retries are set to the default.")
			{
				REQUIRE(parsed.sync_opts.retries == 3);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies a number of retries.")
	{
		constexpr int argc = 4;
		char const* argv[]{ "msync", subcommand, "-r", "10" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the correct number of retries is set")
			{
				REQUIRE(parsed.sync_opts.retries == 10);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies getting only.")
	{
		constexpr int argc = 3;
		char const* argv[]{ "msync", subcommand, "-g" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.get);
				REQUIRE_FALSE(parsed.sync_opts.send);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies sending only.")
	{
		constexpr int argc = 3;
		const char* arg = GENERATE(as<const char*>{}, "-s", "--send", "--send-only");
		char const* argv[]{ "msync", subcommand, arg };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.send);
				REQUIRE_FALSE(parsed.sync_opts.get);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies an account and number of retries with the long options.")
	{
		constexpr int argc = 6;
		char const* argv[]{ "msync", subcommand, "--retries", "15", "--account", "coolfella" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "coolfella");
			}

			THEN("the correct number of retries is set")
			{
				REQUIRE(parsed.sync_opts.retries == 15);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies an account, get only, and number of retries with the long options.")
	{
		constexpr int argc = 7;
		char const* argv[]{ "msync", subcommand, "--get-only", "--retries", "15", "--account", "coolfella" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is set")
			{
				REQUIRE(parsed.account == "coolfella");
			}

			THEN("the correct number of retries is set")
			{
				REQUIRE(parsed.sync_opts.retries == 15);
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.get);
				REQUIRE_FALSE(parsed.sync_opts.send);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies receive only.")
	{
		const char* arg = GENERATE(as<const char*>{}, "-g", "--get", "--recv", "--get-only", "--recv-only");
		std::array<char const*, 3> argv{ "msync", subcommand, arg };

		CAPTURE(argv);

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the defaults are set correctly")
			{
				REQUIRE(parsed.sync_opts.retries == 3);
				REQUIRE(parsed.sync_opts.per_call == 0);
				REQUIRE(parsed.sync_opts.max_requests == 0);
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.get);
				REQUIRE_FALSE(parsed.sync_opts.send);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies receive only and a number of posts per call.")
	{
		const char* arg = GENERATE(as<const char*>{}, "-g", "--get", "--recv", "--get-only", "--recv-only");
		const char* posts = GENERATE(as<const char*>{}, "-p", "--posts");
		std::array<char const*, 5> argv{ "msync", subcommand, get, posts, "50" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the defaults are set correctly")
			{
				REQUIRE(parsed.sync_opts.retries == 3);
				REQUIRE(parsed.sync_opts.max_requests == 0);
			}

			THEN("the non-defaults are set correctly")
			{
				REQUIRE(parsed.sync_opts.per_call == 50);
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.get);
				REQUIRE_FALSE(parsed.sync_opts.send);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line that says 'sync' and specifies a number of posts per call and maximum requests.")
	{
		const char* posts = GENERATE(as<const char*>{}, "-p", "--posts");
		const char* max = GENERATE(as<const char*>{}, "-m", "--max-requests");
		std::array<char const*, 6> argv{ "msync", subcommand,  posts, "60", max, "100" };

		CAPTURE(argv);

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the defaults are set correctly")
			{
				REQUIRE(parsed.sync_opts.retries == 3);
			}

			THEN("the non-defaults are set correctly")
			{
				REQUIRE(parsed.sync_opts.per_call == 60);
				REQUIRE(parsed.sync_opts.max_requests == 100);
			}

			THEN("the correct options are set")
			{
				REQUIRE(parsed.sync_opts.get);
				REQUIRE(parsed.sync_opts.send);
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}

		GIVEN("A command line that says 'sync' and specifies receive only and all the options.")
		{
		const char* arg = GENERATE(as<const char*>{}, "-g", "--get", "--recv", "--get-only", "--recv-only");
			const char* posts = GENERATE(as<const char*>{}, "-p", "--posts");
			const char* max = GENERATE(as<const char*>{}, "-m", "--max-requests");
			const char* retry = GENERATE(as<const char*>{}, "-r", "--retries");
			const char* account = GENERATE(as<const char*>{}, "-a", "--account");
			std::array<char const*, 11> argv{ "msync", subcommand, get, retry, "200", max, "62", posts, "1000", account, "cool@folks.egg" };

			WHEN("the command line is parsed")
			{
				const auto& parsed = parse((int)argv.size(), argv.data());

				THEN("the selected mode is sync")
				{
					REQUIRE(parsed.selected == mode::sync);
				}

				THEN("the account is set")
				{
					REQUIRE(parsed.account == "cool@folks.egg");
				}

				THEN("the non-defaults are set correctly")
				{
					REQUIRE(parsed.sync_opts.retries == 200);
					REQUIRE(parsed.sync_opts.max_requests == 62);
					REQUIRE(parsed.sync_opts.per_call == 1000);
				}

				THEN("the correct options are set")
				{
					REQUIRE(parsed.sync_opts.get);
					REQUIRE_FALSE(parsed.sync_opts.send);
				}

				THEN("the parse is good")
				{
					REQUIRE(parsed.okay);
				}
			}
		}
	}
}

SCENARIO("The command line parser correctly parses when the user wants to interact with the queue.")
{
	const auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
	GIVEN("A command line that just says 'queue'")
	{
		constexpr int argc = 2;
		char const* argv[]{ "msync", qcommand };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is bad.")
			{
				REQUIRE_FALSE(result.okay);
			}
		}
	}

	GIVEN("A command line that adds a bunch of things to the post ID queues.")
	{
		const auto commandtype = GENERATE(std::make_pair("fav", api_route::fav), std::make_pair("boost", api_route::boost), std::make_pair("context", api_route::context));
		constexpr int argc = 6;
		char const* argv[]{ "msync", qcommand, commandtype.first, "12345", "6789", "123FQ43" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected.")
			{
				REQUIRE(result.queue_opt.selected == commandtype.second);
			}

			THEN("the post IDs are parsed.")
			{
				REQUIRE(result.queue_opt.queued == std::vector<std::string>{"12345", "6789", "123FQ43"});
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::add);
			}
		}
	}

	GIVEN("A command line that removes a bunch of things from the boost queue.")
	{
		const auto commandtype = GENERATE(std::make_pair("fav", api_route::fav), std::make_pair("boost", api_route::boost), std::make_pair("context", api_route::context));
		const auto opt = GENERATE(as<const char*>{}, "-r", "--remove", "r", "remove");
		constexpr int argc = 7;
		char const* argv[]{ "msync", qcommand, opt, commandtype.first, "12345", "6789", "bwingus" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == commandtype.second);
			}

			THEN("the post IDs are parsed.")
			{
				REQUIRE(result.queue_opt.queued == std::vector<std::string>{"12345", "6789", "bwingus"});
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::remove);
			}
		}
	}

	GIVEN("A command line that enqueues a post")
	{
		constexpr int argc = 4;
		char const* argv[]{ "msync", qcommand, "post", "msync.post" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == api_route::post);
			}

			THEN("the post IDs are parsed.")
			{
				REQUIRE(result.queue_opt.queued == std::vector<std::string>{"msync.post"});
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::add);
			}
		}
	}

	GIVEN("A command line that clears the post queue.")
	{
		const auto opt = GENERATE(as<const char*>{}, "-c", "--clear", "c", "clear");
		constexpr int argc = 4;
		char const* argv[]{ "msync", qcommand, opt, "post" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == api_route::post);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::clear);
			}
		}
	}

	GIVEN("A command line that prints the current queue.")
	{
		constexpr int argc = 3;
		char const* argv[]{ "msync", qcommand, "print" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::print);
			}
		}
	}

}

SCENARIO("The command line parser recognizes when the user requests yeehaw.")
{
	GIVEN("A command line requesting yeehaw.")
	{
		char const* argv[] { "msync", "yeehaw" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(2, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::yeehaw);
			}
		}
	}

	GIVEN("A command line requesting a verbose yeehaw.")
	{
		const auto verb = GENERATE("-v", "--verbose");

		char const* argv[] { "msync", "yeehaw", verb };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(3, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::yeehaw);
			}
		}
	}
}

SCENARIO("The command line parser recognizes when the user requests msync's version.")
{
	GIVEN("A command line requesting the version.")
	{
		const auto version = GENERATE(as<const char*>{}, "version", "--version");

		char const* argv[] { "msync", version };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(2, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::version);
			}
		}
	}
}

SCENARIO("The command line parser recognizes when the user requests msync's license.")
{
	GIVEN("A command line requesting the license.")
	{
		const auto license = GENERATE(as<const char*>{}, "license", "--license");

		char const* argv[] { "msync", license };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(2, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::license);
			}
		}
	}
}

SCENARIO("The command line parser recognizes when the user requests msync's config location.")
{
	GIVEN("A command line requesting the location.")
	{
		char const* argv[] { "msync", "location" };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(2, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::location);
			}
		}
	}

	GIVEN("A command line requesting the location verbosely.")
	{
		const auto verb = GENERATE("-v", "--verbose");
		char const* argv[] { "msync", "location", verb };

		WHEN("the command line is parsed")
		{
			const auto& result = parse(3, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.selected == mode::location);
			}
		}
	}
}

SCENARIO("The command line parser recognizes when the user wants help.")
{
	GIVEN("A command line that says 'help'.")
	{
		constexpr int argc = 2;
		char const* argv[]{ "msync", "help" };

		WHEN("the command line is parsed")
		{
			const auto& parsed = parse(argc, argv);

			THEN("the selected mode is help")
			{
				REQUIRE(parsed.selected == mode::help);
			}

			THEN("the account is not set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}
}
