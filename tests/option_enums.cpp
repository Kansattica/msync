#include <catch2/catch.hpp>

#include "../lib/options/option_enums.hpp"

CATCH_REGISTER_ENUM(list_operations, list_operations::add, list_operations::remove, list_operations::clear)

SCENARIO("list_operations stringify and parse properly.", "[option_enums]")
{
    Catch::StringMaker<list_operations> sm;
    GIVEN("A list_operation")
    {
        auto val = GENERATE(list_operations::add, list_operations::remove, list_operations::clear);
        WHEN("that list_operation is looked up in its array")
        {
            auto result = LIST_OPERATION_NAMES[static_cast<int>(val)];
            THEN("the corresponding string is the correct one.")
            {
                REQUIRE(result == sm.convert(val));
            }

            AND_WHEN("the looked-up string is parsed")
            {
                auto parsedval = parse_enum<list_operations>(result[0]);

                THEN("it matches the original.")
                {
                    REQUIRE(parsedval == val);
                }
            }
        }
    }

    GIVEN("A list_operations type")
    {
        THEN("Its array has an entry for each value.")
        {
            STATIC_REQUIRE(LIST_OPERATION_NAMES.size() == static_cast<int>(list_operations::clear) + 1);
        }
    }
}

CATCH_REGISTER_ENUM(sync_settings, sync_settings::dont_sync, sync_settings::newest_first, sync_settings::oldest_first)

SCENARIO("sync_settings stringify properly.", "[option_enums]")
{
    Catch::StringMaker<sync_settings> sm;
    GIVEN("A sync_setting")
    {
        auto val = GENERATE(sync_settings::dont_sync, sync_settings::newest_first, sync_settings::oldest_first);
        WHEN("that sync_setting is looked up in its array")
        {
            auto result = SYNC_SETTING_NAMES[static_cast<int>(val)];
            THEN("the corresponding string is the correct one.")
            {
                REQUIRE(result == sm.convert(val));
            }

            AND_WHEN("the looked-up string is parsed")
            {
                auto parsedval = parse_enum<sync_settings>(result[0]);

                THEN("it matches the original.")
                {
                    REQUIRE(parsedval == val);
                }
            }
        }
    }

    GIVEN("A sync_settings type")
    {
        THEN("Its array has an entry for each value.")
        {
            STATIC_REQUIRE(SYNC_SETTING_NAMES.size() == static_cast<int>(sync_settings::oldest_first) + 1);
        }
    }
}

CATCH_REGISTER_ENUM(user_option, user_option::account_name, user_option::instance_url, user_option::auth_code,
                    user_option::access_token, user_option::client_secret, user_option::client_id, user_option::pull_home, user_option::pull_dms, user_option::pull_notifications)

SCENARIO("user_option values stringify properly.", "[option_enums]")
{
    Catch::StringMaker<user_option> sm;
    GIVEN("A user_option")
    {
        auto val = GENERATE(user_option::account_name, user_option::instance_url, user_option::auth_code,
                            user_option::access_token, user_option::client_secret, user_option::client_id, user_option::pull_home, user_option::pull_dms, user_option::pull_notifications);

        WHEN("that user_option is looked up in its array")
        {
            auto result = USER_OPTION_NAMES[static_cast<int>(val)];
            THEN("the corresponding string is the correct one.")
            {
                REQUIRE(result == sm.convert(val));
            }
        }
    }

    GIVEN("A user_options type")
    {
        THEN("Its array has an entry for each value.")
        {
            STATIC_REQUIRE(USER_OPTION_NAMES.size() == static_cast<int>(user_option::pull_notifications) + 1);
        }
    }
}