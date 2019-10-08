#include <catch2/catch.hpp>

#include <vector>
#include <algorithm>

#include "../console/optionparsing/parseoptions.hpp"

SCENARIO("The command line parser recognizes when the user wants to start a new account.")
{
    GIVEN("A command line that doesn't specify an account.")
    {
        int argc = 2;
        char const* argv[]{"msync", "new"};

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
        char const* argv[]{"msync", "new", "-a", "regular@folks.egg"};

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
        char const* argv[]{"msync", "config", "accesstoken", "sometoken"};

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
        char const* argv[]{"msync", "config", "clientsecret", "asecret!", "-a", "jerk@fun.website"};

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
        char const* argv[]{"msync", "config", "accountname"};

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
        char const* argv[]{"msync", "config", "instanceurl", "-a", "niceperson@impolite.egg"};

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
        char const* argv[]{"msync", "config", "showall"};

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
        char const* argv[]{"msync", "config", "showall", "-a", "regular@folks.egg"};

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
        char const* argv[]{"msync", "config", "list", "add", "somelist"};

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
        char const* argv[]{"msync", "config", "list", "remove", "anotherlist", "-a", "coolfriend"};

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
        char const* argv[]{"msync", "config", "sync", "home", "oldest", "-a", "coolerfriend"};

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
        char const* argv[]{"msync", "config", "sync", "dms", "newest"};

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
        char const* argv[]{"msync", "config", "sync", "notifications", "off", "-a", "coolestfriend"};

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
        char const* argv[]{"msync", "sync"};

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
        char const* argv[]{"msync", "sync", "-r", "10"};

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
		opt.order = 0;
		opt.options.push_back("-f");
		opt.options.push_back("someattach");
		expected.post.attachments.push_back("someattach");
		break;
	case 1:
		opt.order = 1;
		opt.options.push_back("-f");
		opt.options.push_back("someotherattach");
		opt.options.push_back("thirdattach");
		expected.post.attachments.push_back("someotherattach");
		expected.post.attachments.push_back("thirdattach");
		break;
	case 2:
		opt.order = 2;
		opt.options.push_back("--attach");
		opt.options.push_back("attacher");
		opt.options.push_back("somefile");
		expected.post.attachments.push_back("attacher");
		expected.post.attachments.push_back("somefile");
		break;
	case 3:
		opt.order = 3;
		opt.options.push_back("--file");
		opt.options.push_back("filey");
		expected.post.attachments.push_back("filey");
		break;
	case 4:
		break;
	}
	return opt;
}


SCENARIO("The command line parser recognizes when the user wants to generate a file.")
{
	GIVEN("A combination of options for the file generator")
	{
		auto combination = GENERATE(range(0, (1 << 4) - 1));
		auto attach = GENERATE(0, 1, 2, 3, 4);

		gen_options expected;
		std::vector<command_line_option> options;

		// pick one of the attachment guys
		{
			auto attachopt = pick_attachment(attach, expected);
			if (attachopt.order != -1)
			{
				options.push_back(std::move(attachopt));
			}
		}

		if (flag_set(combination, 0))
		{
			command_line_option opt;
			opt.order = 4;
			if (combination % 2 == 0)
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
			opt.order = 5;
			if (combination % 2 == 0)
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
			opt.order = 6;
			if (combination % 2 == 0)
				opt.options.push_back("-c");
			else
				opt.options.push_back("--content-warning");

			opt.options.push_back("there's content in here!");
			expected.post.content_warning  = "there's content in here!";
			options.push_back(std::move(opt));
		}

		std::sort(options.begin(), options.end());
		std::vector<const char*> argv;

		WHEN("the command line is parsed")
		{
			do
			{
				if (flag_set(combination, 3))
					argv = { "msync", "gen" };
				else
					argv = { "msync", "generate" };

				for (auto& option : options)
				{
					argv.insert(argv.end(), option.options.begin(), option.options.end());
				}

				auto parsed = parse(argv.size(), argv.data());

				THEN("the parse is good")
				{
					REQUIRE(parsed.okay);
				}

				THEN("the correct mode is set")
				{
					REQUIRE(parsed.selected == mode::gen);
				}

				THEN("the account is not set")
				{
					REQUIRE(parsed.account.empty());
				}

				THEN("the options are set as expected")
				{
					REQUIRE(expected.filename == parsed.gen_opt.filename);
					REQUIRE(expected.post.attachments == parsed.gen_opt.post.attachments);
					REQUIRE(std::is_permutation(expected.post.attachments.begin(), expected.post.attachments.end(),
						parsed.gen_opt.post.attachments.begin(), parsed.gen_opt.post.attachments.end()));
					REQUIRE(expected.post.content_warning == parsed.gen_opt.post.content_warning);
					REQUIRE(expected.post.reply_to_id == parsed.gen_opt.post.reply_to_id);
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
