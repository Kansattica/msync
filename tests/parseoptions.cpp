#include <catch2/catch.hpp>

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

SCENARIO("The command line parser correctly parses when the user wants to queue.")
{
    GIVEN("A command line that just says 'queue'")
    {
        int argc = 2;
        char const* argv[]{"msync", "queue"};

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
        int argc = 6;
        char const* argv[]{"msync", "queue", "fav", "12345", "6789", "123FQ43"};

        WHEN("the command line is parsed")
        {
            auto result = parse(argc, argv);

            THEN("the parse is good.")
            {
                REQUIRE(result.okay);
            }

            THEN("the correct queue is selected")
            {
                REQUIRE(result.queue_opt.selected == queues::fav);
            }

            THEN("the post IDs are parsed.")
            {
                REQUIRE(result.queue_opt.queued == std::vector<std::string>{"12345", "6789", "123FQ43"});
            }
        }
    }
}

SCENARIO("The command line parser recognizes when the user wants help.")
{
    GIVEN("A command line that says 'help'.")
    {
        int argc = 2;
        char const* argv[]{"msync", "help"};

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