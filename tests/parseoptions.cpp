#include <catch2/catch.hpp>

#include <vector>
#include <algorithm>

#include "../console/optionparsing/parseoptions.hpp"

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
		char const* argv[]{ "msync", "config", "accesstoken", "sometoken" };

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
		char const* argv[]{ "msync", "config", "clientsecret", "asecret!", "-a", "jerk@fun.website" };

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
		char const* argv[]{ "msync", "config", "accountname" };

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
		char const* argv[]{ "msync", "config", "instanceurl", "-a", "niceperson@impolite.egg" };

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
        char const* argv[]{"msync", "sync", "-g"};

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
        char const* argv[]{"msync", "sync", "-s"};

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
        char const* argv[]{"msync", "sync", "--retries", "15", "--account", "coolfella"};

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
        char const* argv[]{"msync", "sync", "--get-only", "--retries", "15", "--account", "coolfella"};

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
		char const* argv[]{ "msync", qcommand, "post", "msync.post"};

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

	GIVEN("A command line that prints the post queue.")
	{
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 4;
		char const* argv[]{ "msync", qcommand, "-p", "post" };

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
				REQUIRE(result.queue_opt.to_do == queue_action::print);
			}
		}
	}

	GIVEN("A command line that prints the post queue with long options.")
	{
		auto qcommand = GENERATE(as<const char*>{}, "queue", "q");
		int argc = 4;
		char const* argv[]{ "msync", qcommand, "--print", "post" };

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
				REQUIRE(result.queue_opt.to_do == queue_action::print);
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
	int order = -1;
	std::vector<const char*> options;

	friend bool operator< (const command_line_option& lhs, const command_line_option& rhs)
	{
		return lhs.order < rhs.order;
	}
};

command_line_option pick_attachment(int number, gen_options& expected)
{
	command_line_option opt;

	switch (number)
	{
	case 0:
		opt.options.push_back("-f");
		opt.options.push_back("someattach");
		expected.post.attachments.push_back("someattach");
		break;
	case 1:
		opt.options.push_back("--attach");
		opt.options.push_back("attacher");
		opt.options.push_back("somefile");
		expected.post.attachments.push_back("attacher");
		expected.post.attachments.push_back("somefile");
		break;
	//case 1:
	//	opt.options.push_back("-f");
	//	opt.options.push_back("someotherattach");
	//	opt.options.push_back("thirdattach");
	//	expected.post.attachments.push_back("someotherattach");
	//	expected.post.attachments.push_back("thirdattach");
	//	break;
	//case 3:
	//	opt.options.push_back("--file");
	//	opt.options.push_back("filey");
	//	expected.post.attachments.push_back("filey");
	//	break;
	case 2:
		break;
	}
	return opt;
}

command_line_option pick_description(int number, gen_options& expected)
{
	command_line_option opt;

	switch (number)
	{
	case 0:
		opt.options.push_back("-d");
		opt.options.push_back("somedescrip");
		expected.post.descriptions.push_back("somedescrip");
		break;
	case 1:
		opt.options.push_back("--description");
		opt.options.push_back("describer");
		opt.options.push_back("some file!");
		expected.post.descriptions.push_back("describer");
		expected.post.descriptions.push_back("some file!");
	//case 1:
	//	opt.options.push_back("-d");
	//	opt.options.push_back("some other description");
	//	opt.options.push_back("thirddescrip");
	//	expected.post.descriptions.push_back("some other description");
	//	expected.post.descriptions.push_back("thirddescrip");
	//	break;
	//	break;
	//case 3:
	//	opt.options.push_back("--description");
	//	opt.options.push_back("some jerk doing a thing.");
	//	expected.post.descriptions.push_back("some jerk doing a thing.");
	//	break;
	case 2:
		break;
	}
	return opt;
}


SCENARIO("The command line parser recognizes when the user wants to generate a file.", "[long_run]")
{
	GIVEN("A combination of options for the file generator")
	{
		// try every combination of bits. note that the ranges are half-open, including the 0 and excluding the maximum.
		const auto combination = GENERATE(range(0, 0b1111 + 1));
		const auto longopt = GENERATE(range(0, 0b11111 + 1));
		const auto attach = GENERATE(0, 1, 2);
		const auto description = GENERATE(0, 1, 2);

		gen_options expected;
		std::vector<command_line_option> options;
		options.reserve(6);

		if (attach != 2)
		{
			options.push_back(pick_attachment(attach, expected));
		}

		if (description != 2)
		{
			options.push_back(pick_description(description, expected));
		}

		if (flag_set(combination, 0))
		{
			command_line_option opt;
			if (flag_set(longopt, 0))
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
			if (flag_set(longopt, 1))
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
			if (flag_set(longopt, 2))
				opt.options.push_back("-c");
			else if (flag_set(longopt, 0)) // I don't really want to make the number of combinations even worse
				opt.options.push_back("--content-warning");
			else
				opt.options.push_back("--cw");

			opt.options.push_back("there's content in here!");
			expected.post.content_warning = "there's content in here!";
			options.push_back(std::move(opt));
		}

		if (flag_set(combination, 3))
		{
			command_line_option opt;
			if (flag_set(longopt, 3))
				opt.options.push_back("-i");
			else
				opt.options.push_back("--reply-id");

			opt.options.push_back("76543");
			expected.post.reply_id = "76543";
			options.push_back(std::move(opt));
		}

		for (int i = 0; i < options.size(); i++)
			options[i].order = i;

		std::vector<const char*> argv;

		WHEN("the command line is parsed")
		{
			do
			{
				argv.clear();
				argv.push_back("msync");
				if (flag_set(longopt, 4))
					argv.push_back("gen");
				else
					argv.push_back("generate");

				for (const auto& option : options)
				{
					argv.insert(argv.end(), option.options.begin(), option.options.end());
				}

				const auto parsed = parse(argv.size(), argv.data());

				THEN("the options are parsed as expected")
				{
					REQUIRE(parsed.okay);
					REQUIRE(parsed.selected == mode::gen);
					REQUIRE(parsed.account.empty());

					REQUIRE(expected.filename == parsed.gen_opt.filename);
					REQUIRE(expected.post.attachments == parsed.gen_opt.post.attachments);
					REQUIRE(expected.post.descriptions == parsed.gen_opt.post.descriptions);
					REQUIRE(expected.post.content_warning == parsed.gen_opt.post.content_warning);
					REQUIRE(expected.post.reply_to_id == parsed.gen_opt.post.reply_to_id);
					REQUIRE(expected.post.reply_id == parsed.gen_opt.post.reply_id);
				}
			} while (std::next_permutation(options.begin(), options.end()));

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
