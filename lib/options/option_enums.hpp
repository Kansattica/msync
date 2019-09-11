#ifndef _ENUM_NAMES_HPP_
#define _ENUM_NAMES_HPP_
#include <array>
#include <string>

enum class list_operations
{
    add,
    remove,
    clear
};

const auto LIST_OPERATION_NAMES =
    std::array<const std::string,
               static_cast<int>(list_operations::clear) + 1>(
        {"add", "remove", "clear"});

enum class sync_settings
{
    off,
    newest_first,
    oldest_first
};

const auto SYNC_SETTING_NAMES =
    std::array<const std::string,
               static_cast<int>(sync_settings::oldest_first) + 1>(
        {"off", "newest_first", "oldest_first"});

enum class user_option
{
    account_name,
    instance_url,
    auth_code,
    access_token,
    client_secret,
    client_id,
    pull_home,
    pull_dms,
    pull_notifications,
};

const auto USER_OPTION_NAMES =
    std::array<const std::string,
               static_cast<int>(user_option::pull_notifications) + 1>(
        {"account_name", "instance_url", "auth_code", "access_token", "client_secret", "client_id",
         "pull_home", "pull_dms", "pull_notifications"});
#endif