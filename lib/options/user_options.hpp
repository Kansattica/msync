#include <string>
#include <vector>
#include <iostream>

#include "filesystem.hpp"

enum class sync_settings
{
    off,
    newest_first,
    oldest_first
};

struct user_options
{
    user_options() = default;
    user_options(fs::path toread);

    std::string account_name;
    std::string instance_url;

    std::string access_token;

    std::vector<std::string> ids_to_favorite;
    std::vector<std::string> ids_to_boost;
    std::vector<std::string> filenames_to_post;

    std::vector<std::string> lists_to_pull;

    sync_settings pull_home = sync_settings::newest_first;
    sync_settings pull_dms = sync_settings::oldest_first;
    sync_settings pull_notifications = sync_settings::oldest_first;
};

enum class user_option
{
    show,
    accesstoken,
    username,
    password,
    clientsecret,
    addlist,
    removelist,
    home,
    dms,
    notifications,
    newaccount
};
