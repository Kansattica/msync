#include <catch2/catch.hpp>

#include "../console/optionparsing/parseoptions.hpp"

SCENARIO("The command line parser recognizes when the user wants to start a new account.")
{
    GIVEN("A command line that doesn't specify an account.")
    {
        int argc = 2;
        char const *argv[] {"msync", "new"};

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
        char const *argv[]{"msync", "new", "-a", "regular@folks.egg"};

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
        char const *argv[]{"msync", "config", "accesstoken", "sometoken"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is config")
            {
                REQUIRE(parsed.selected == mode::config);
            }

            THEN("the correct option will be changed")
            {
                REQUIRE(parsed.toset == user_option::accesstoken);
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
        char const *argv[]{"msync", "config", "clientsecret", "asecret!", "-a", "jerk@fun.website"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is config")
            {
                REQUIRE(parsed.selected == mode::config);
            }

            THEN("the correct option will be changed")
            {
                REQUIRE(parsed.toset == user_option::clientsecret);
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
        char const *argv[]{"msync", "config", "username"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is showopt")
            {
                REQUIRE(parsed.selected == mode::showopt);
            }

            THEN("the correct option will be read")
            {
                REQUIRE(parsed.toset == user_option::username);
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
        char const *argv[]{"msync", "config", "password", "-a", "niceperson@impolite.egg"};

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
                REQUIRE(parsed.toset == user_option::password);
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
        char const *argv[]{"msync", "config", "showall"};

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
        char const *argv[]{"msync", "config", "showall", "-a", "regular@folks.egg"};

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
        char const *argv[]{"msync", "config", "list", "add", "somelist"};

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
        char const *argv[]{"msync", "config", "list", "remove", "anotherlist", "-a", "coolfriend"};

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
        char const *argv[]{"msync", "config", "sync", "home", "oldest", "-a", "coolerfriend"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is configsync")
            {
                REQUIRE(parsed.selected == mode::configsync);
            }

            THEN("the correct sync location is set")
            {
                REQUIRE(parsed.toset == user_option::home);
            }

            THEN("the correct sync operation is set")
            {
                REQUIRE(parsed.syncset == sync_settings::oldest_first);
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
        char const *argv[]{"msync", "config", "sync", "dms", "newest"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is configsync")
            {
                REQUIRE(parsed.selected == mode::configsync);
            }

            THEN("the correct sync location is set")
            {
                REQUIRE(parsed.toset == user_option::dms);
            }

            THEN("the correct sync operation is set")
            {
                REQUIRE(parsed.syncset == sync_settings::newest_first);
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
        char const *argv[]{"msync", "config", "sync", "notifications", "off", "-a", "coolestfriend"};

        WHEN("the command line is parsed")
        {
            auto parsed = parse(argc, argv);

            THEN("the selected mode is configsync")
            {
                REQUIRE(parsed.selected == mode::configsync);
            }

            THEN("the correct sync location is set")
            {
                REQUIRE(parsed.toset == user_option::notifications);
            }

            THEN("the correct sync operation is set")
            {
                REQUIRE(parsed.syncset == sync_settings::off);
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

SCENARIO("The command line parser recognizes when the user wants help.")
{
    GIVEN("A command line that says 'help'.")
    {
        int argc = 2;
        char const *argv[]{"msync", "help"};

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