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
		int argc = 2;
		char const* argv[]{ "msync", "new" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is new user")
			{
				REQUIRE(parsed.selected == mode::newuser);
			}

			THEN("account is not set")
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
		int argc = 4;
		char const* argv[]{ "msync", "new", "-a", "regular@folks.egg" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is new user")
			{
				REQUIRE(parsed.selected == mode::newuser);
			}

			THEN("account is set")
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
		int argc = 4;
		char const* argv[]{ "msync", "config", "access_token", "sometoken" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is not set")
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
		int argc = 6;
		char const* argv[]{ "msync", "config", "client_secret", "asecret!", "-a", "jerk@fun.website" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
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
		int argc = 3;
		char const* argv[]{ "msync", "config", "account_name" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is not set")
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
		int argc = 5;
		char const* argv[]{ "msync", "config", "instance_url", "-a", "niceperson@impolite.egg" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
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
		int argc = 3;
		char const* argv[]{ "msync", "config", "showall" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is showallopt")
			{
				REQUIRE(parsed.selected == mode::showallopt);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("account is not set")
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
		int argc = 5;
		char const* argv[]{ "msync", "config", "showall", "-a", "regular@folks.egg" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is showallopt")
			{
				REQUIRE(parsed.selected == mode::showallopt);
			}

			THEN("the option is not set")
			{
				REQUIRE(parsed.optionval.empty());
			}

			THEN("account is set")
			{
				REQUIRE(parsed.account == "regular@folks.egg");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line adding a list to be pulled.")
	{
		int argc = 5;
		char const* argv[]{ "msync", "config", "list", "add", "somelist" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is not set")
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
		int argc = 7;
		char const* argv[]{ "msync", "config", "list", "remove", "anotherlist", "-a", "coolfriend" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
			{
				REQUIRE(parsed.account == "coolfriend");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line specifying that the home timeline should be synced oldest first.")
	{
		int argc = 7;
		char const* argv[]{ "msync", "config", "sync", "home", "oldest", "-a", "coolerfriend" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
			{
				REQUIRE(parsed.account == "coolerfriend");
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line specifying that DMs should be synced newest first.")
	{
		int argc = 5;
		char const* argv[]{ "msync", "config", "sync", "dms", "newest" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
			{
				REQUIRE(parsed.account.empty());
			}

			THEN("the parse is good")
			{
				REQUIRE(parsed.okay);
			}
		}
	}

	GIVEN("A command line specifying that the notifications timeline should not be synced.")
	{
		int argc = 7;
		char const* argv[]{ "msync", "config", "sync", "notifications", "off", "-a", "coolestfriend" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

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

			THEN("account is set")
			{
				REQUIRE(parsed.account == "coolestfriend");
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
	GIVEN("A command line that says 'sync'.")
	{
		int argc = 2;
		char const* argv[]{ "msync", "sync" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		int argc = 4;
		char const* argv[]{ "msync", "sync", "-r", "10" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		int argc = 3;
		char const* argv[]{ "msync", "sync", "-g" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		int argc = 3;
		char const* argv[]{ "msync", "sync", "-s" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		int argc = 6;
		char const* argv[]{ "msync", "sync", "--retries", "15", "--account", "coolfella" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is set")
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
		int argc = 7;
		char const* argv[]{ "msync", "sync", "--get-only", "--retries", "15", "--account", "coolfella" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is set")
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
		const char* arg = GENERATE(as<const char*>{}, "-g", "--get-only", "--recv-only");
		std::array<char const*, 3> argv{ "msync", "sync", arg };

		WHEN("the command line is parsed")
		{
			auto parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		const char* get = GENERATE(as<const char*>{}, "-g", "--get-only", "--recv-only");
		const char* posts = GENERATE(as<const char*>{}, "-p", "--posts");
		std::array<char const*, 5> argv{ "msync", "sync", get, posts, "50" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
		std::array<char const*, 6> argv{ "msync", "sync",  posts, "60", max, "100" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse((int)argv.size(), argv.data());

			THEN("the selected mode is sync")
			{
				REQUIRE(parsed.selected == mode::sync);
			}

			THEN("account is not set")
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
			const char* get = GENERATE(as<const char*>{}, "-g", "--get-only", "--recv-only");
			const char* posts = GENERATE(as<const char*>{}, "-p", "--posts");
			const char* max = GENERATE(as<const char*>{}, "-m", "--max-requests");
			const char* retry = GENERATE(as<const char*>{}, "-r", "--retries");
			const char* account = GENERATE(as<const char*>{}, "-a", "--account");
			std::array<char const*, 11> argv{ "msync", "sync", get, retry, "200", max, "62", posts, "1000", account, "cool@folks.egg" };

			WHEN("the command line is parsed")
			{
				auto parsed = parse((int)argv.size(), argv.data());

				THEN("the selected mode is sync")
				{
					REQUIRE(parsed.selected == mode::sync);
				}

				THEN("account is set")
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
	GIVEN("A command line that just says 'queue'")
	{
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 2;
		char const* argv[]{ "msync", qcommand };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

			THEN("the parse is bad.")
			{
				REQUIRE_FALSE(result.okay);
			}
		}
	}

	GIVEN("A command line that adds a bunch of things to the fav queue.")
	{
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 6;
		char const* argv[]{ "msync", qcommand, "fav", "12345", "6789", "123FQ43" };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected.")
			{
				REQUIRE(result.queue_opt.selected == queues::fav);
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
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 7;
		char const* argv[]{ "msync", qcommand, "-r", "boost", "12345", "6789", "bwingus" };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == queues::boost);
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
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 4;
		char const* argv[]{ "msync", qcommand, "post", "msync.post" };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == queues::post);
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
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 4;
		char const* argv[]{ "msync", qcommand, "-c", "post" };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

			THEN("the parse is good.")
			{
				REQUIRE(result.okay);
			}

			THEN("the correct queue is selected")
			{
				REQUIRE(result.queue_opt.selected == queues::post);
			}

			THEN("the correct action is selected.")
			{
				REQUIRE(result.queue_opt.to_do == queue_action::clear);
			}
		}
	}

	GIVEN("A command line that prints the current queue.")
	{
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 3;
		char const* argv[]{ "msync", qcommand, "print" };

		WHEN("the command line is parsed")
		{
			auto result = parse(argc, argv);

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
			const auto result = parse(2, argv);

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
			const auto result = parse(3, argv);

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
			const auto result = parse(2, argv);

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
			const auto result = parse(2, argv);

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

bool flag_set(int combo, int position)
{
	return combo & (1 << position);
}

struct command_line_option
{
	std::vector<const char*> options;
	unsigned int attachment_order = 0;
	unsigned int description_order = 0;
	unsigned int order = 1000;

	friend bool operator< (const command_line_option& lhs, const command_line_option& rhs)
	{
		return lhs.order < rhs.order;
	}
};


void pick_attachment(int number, gen_options& expected, std::vector<command_line_option>& options)
{
	switch (number)
	{
	case 0:
		options.push_back(command_line_option{ {"-f", "someattach"} });
		expected.post.attachments.push_back("someattach");
		break;
	case 1:
		options.push_back(command_line_option{ {"--attach", "attacher"}, 1 });
		options.push_back(command_line_option{ { "-f", "somefile"}, 2 });
		expected.post.attachments.push_back("attacher");
		expected.post.attachments.push_back("somefile");
		break;
	case 2:
		options.push_back(command_line_option{ {"--file", "filey"} });
		expected.post.attachments.push_back("filey");
		break;
	case 3:
		break;
		//case 1:
		//	opt.options.push_back("-f");
		//	opt.options.push_back("someotherattach");
		//	opt.options.push_back("thirdattach");
		//	expected.post.attachments.push_back("someotherattach");
		//	expected.post.attachments.push_back("thirdattach");
		//	break;
	}
}

void pick_description(int number, gen_options& expected, std::vector<command_line_option>& options)
{
	switch (number)
	{
	case 0:
		options.push_back(command_line_option{ {"-d", "somedescrip"} });
		expected.post.descriptions.push_back("somedescrip");
		break;
	case 1:
		options.push_back(command_line_option{ {"--description", "describer"}, 0, 1 });
		options.push_back(command_line_option{ {"-d", "some file!"}, 0, 2 });
		expected.post.descriptions.push_back("describer");
		expected.post.descriptions.push_back("some file!");
		break;
		//case 1:
		//	opt.options.push_back("-d");
		//	opt.options.push_back("some other description");
		//	opt.options.push_back("thirddescrip");
		//	expected.post.descriptions.push_back("some other description");
		//	expected.post.descriptions.push_back("thirddescrip");
		//	break;
		//case 3:
		//	opt.options.push_back("--description");
		//	opt.options.push_back("some jerk doing a thing.");
		//	expected.post.descriptions.push_back("some jerk doing a thing.");
		//	break;
	case 2:
		break;
	}
}

auto pick_visibility()
{
	switch (zero_to_n(7))
	{
	case 0:
		return std::make_pair("", visibility::default_vis);
	case 1:
		return std::make_pair("default", visibility::default_vis);
	case 2:
		return std::make_pair("public", visibility::pub);
	case 3:
		return std::make_pair("private", visibility::priv);
	case 4:
		return std::make_pair("followersonly", visibility::priv);
	case 5:
		return std::make_pair("unlisted", visibility::unlisted);
	case 6:
		return std::make_pair("dm", visibility::direct);
	case 7:
		return std::make_pair("direct", visibility::direct);
	}

	FAIL("Hey, the visibility picker screwed up.");
	return std::make_pair("Well, this shouldn't happen.", visibility::default_vis);
}

template <typename get_field>
bool should_reverse(const std::vector<command_line_option>& options, get_field func)
{
	const auto found = std::find_if(options.begin(), options.end(), [func](const auto& opt) { return func(opt) != 0; });
	if (found == options.end() || func(*found) == 1)
		return false;
	return true;
}

template <typename T>
std::vector<T> reversed(const std::vector<T>& vec)
{
	return std::vector<T> { vec.rbegin(), vec.rend() };
}

void check_parse(std::vector<const char*>& argv, const std::vector<command_line_option>& options, const gen_options& expected)
{
	if (flip_coin())
		argv = { "msync", "gen" };
	else
		argv = { "msync", "generate" };

	for (const auto& option : options)
	{
		argv.insert(argv.end(), option.options.begin(), option.options.end());
	}

	const auto parsed = parse((int)argv.size(), argv.data());

	THEN("the options are parsed as expected")
	{
		CAPTURE(argv);
		REQUIRE(parsed.okay);
		REQUIRE(parsed.selected == mode::gen);
		REQUIRE(parsed.account.empty());

		REQUIRE(expected.filename == parsed.gen_opt.filename);
		REQUIRE(expected.post.text == parsed.gen_opt.post.text);
		REQUIRE(expected.post.vis == parsed.gen_opt.post.vis);
		REQUIRE(expected.post.content_warning == parsed.gen_opt.post.content_warning);
		REQUIRE(expected.post.reply_to_id == parsed.gen_opt.post.reply_to_id);
		REQUIRE(expected.post.reply_id == parsed.gen_opt.post.reply_id);

		// basically, it's possible for these to get permuted so that they're not in the original order.
		if (should_reverse(options, [](const command_line_option& opt) { return opt.attachment_order; }))
			REQUIRE(reversed(expected.post.attachments) == parsed.gen_opt.post.attachments);
		else
			REQUIRE(expected.post.attachments == parsed.gen_opt.post.attachments);

		if (should_reverse(options, [](const command_line_option& opt) { return opt.description_order; }))
			REQUIRE(reversed(expected.post.descriptions) == parsed.gen_opt.post.descriptions);
		else
			REQUIRE(expected.post.descriptions == parsed.gen_opt.post.descriptions);
	}
}

SCENARIO("The command line parser recognizes when the user wants to generate a file.", "[long_run][long_run_parseopts]")
{
	GIVEN("A combination of options for the file generator")
	{
		// try every combination of bits. note that the ranges are half-open, including the 0 and excluding the maximum.
		// this test isn't as exhaustive as it could be, because if it was, it'd take forever to run
		const auto combination = GENERATE(range(0, 0b11111 + 1));
		const auto attach = GENERATE(0, 1, 2, 3);
		const auto description = GENERATE(0, 1, 2);
		const auto vis = pick_visibility();

		gen_options expected;

		// this guy is going to be refilled and emptied a bunch
		// make 'em static and clear it every time to keep the capacity
		static std::vector<command_line_option> options;
		options.clear();

		if (vis.first[0] != '\0')
		{
			command_line_option opt;

			switch (zero_to_n(2))
			{
			case 0:
				opt.options.push_back("-p");
				break;
			case 1:
				opt.options.push_back("--privacy");
				break;
			case 2:
				opt.options.push_back("--visibility");
				break;
			}

			opt.options.push_back(vis.first);
			expected.post.vis = vis.second;
			options.push_back(std::move(opt));
		}

		if (attach != 3)
		{
			pick_attachment(attach, expected, options);
		}

		if (description != 2)
		{
			pick_description(description, expected, options);
		}

		if (flag_set(combination, 0))
		{
			command_line_option opt;
			if (flip_coin())
				opt.options.push_back("-o");
			else
				opt.options.push_back("--output");

			opt.options.push_back("filename");
			expected.filename = "filename";
			options.push_back(std::move(opt));
		}

		if (flag_set(combination, 1))
		{
			command_line_option opt;
			if (flip_coin())
				opt.options.push_back("-r");
			else
				opt.options.push_back("--reply-to");

			opt.options.push_back("1234567");
			expected.post.reply_to_id = "1234567";
			options.push_back(std::move(opt));
		}

		if (flag_set(combination, 2))
		{
			command_line_option opt;

			switch (zero_to_n(2))
			{
			case 0:
				opt.options.push_back("-c");
				break;
			case 1:
				opt.options.push_back("--content-warning");
				break;
			case 2:
				opt.options.push_back("--cw");
				break;
			}

			opt.options.push_back("there's content in here!");
			expected.post.content_warning = "there's content in here!";
			options.push_back(std::move(opt));
		}

		if (flag_set(combination, 3))
		{
			command_line_option opt;
			if (flip_coin())
				opt.options.push_back("-i");
			else
				opt.options.push_back("--reply-id");

			opt.options.push_back("76543");
			expected.post.reply_id = "76543";
			options.push_back(std::move(opt));
		}

		if (flag_set(combination, 4))
		{
			command_line_option opt;
			switch (zero_to_n(2))
			{
			case 0:
				opt.options.push_back("-b");
				break;
			case 1:
				opt.options.push_back("--body");
				break;
			case 2:
				opt.options.push_back("--content");
				break;
			}

			opt.options.push_back("@someguy@website.com");
			expected.post.text = "@someguy@website.com";
			options.push_back(std::move(opt));
		}

		for (unsigned int i = 0; i < options.size(); i++)
			options[i].order = i;


		WHEN("the command line is parsed")
		{
			// static and doing the pass-by-mutable-ref thing because there's really no sense in 
			// freeing and reallocating for every test case
			// check_parse clears it every time, but keeps the capacity
			static std::vector<const char*> argv;

			// exhaustively trying every permutation takes far too long once you get past 7 or 8
			// so if there's more than that, randomly shuffle instead
			if (options.size() <= 7)
			{
				do
				{
					check_parse(argv, options, expected);
				} while (std::next_permutation(options.begin(), options.end()));
			}
			else
			{
				static std::minstd_rand g(std::random_device{}());
				// shuffle once because shuffling is slow
				std::shuffle(options.begin(), options.end(), g);
				for (int i = 0; i < 6000; i++)
				{
					check_parse(argv, options, expected);
					std::next_permutation(options.begin(), options.end());
				}
			}

		}
	}
}

SCENARIO("The command line parser recognizes when the user wants help.")
{
	GIVEN("A command line that says 'help'.")
	{
		int argc = 2;
		char const* argv[]{ "msync", "help" };

		WHEN("the command line is parsed")
		{
			auto parsed = parse(argc, argv);

			THEN("the selected mode is help")
			{
				REQUIRE(parsed.selected == mode::help);
			}

			THEN("account is not set")
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
