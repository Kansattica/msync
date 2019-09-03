#include <string>
#include <vector>

struct user_options
{
    // including instance name, like name@instance.social
    std::string account_name;
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

enum class sync_settings
{
    off,
    newest_first,
    oldest_first
};