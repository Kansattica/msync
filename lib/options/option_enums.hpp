#ifndef _ENUM_NAMES_HPP_
#define _ENUM_NAMES_HPP_
#include <array>
#include <string_view>

template <typename ToParse>
ToParse parse_enum(const char first);

enum class list_operations
{
    add,
    remove,
    clear
};

constexpr auto LIST_OPERATION_NAMES =
    std::array<std::string_view,
               static_cast<int>(list_operations::clear) + 1>(
        {"add", "remove", "clear"});

enum class sync_settings
{
    dont_sync,
    newest_first,
    oldest_first
};

constexpr auto SYNC_SETTING_NAMES =
    std::array<std::string_view,
               static_cast<int>(sync_settings::oldest_first) + 1>(
        {"dont_sync", "newest_first", "oldest_first"});

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

constexpr auto USER_OPTION_NAMES =
    std::array<std::string_view,
               static_cast<int>(user_option::pull_notifications) + 1>(
        {"account_name", "instance_url", "auth_code", "access_token", "client_secret", "client_id",
         "pull_home", "pull_dms", "pull_notifications"});
#endif