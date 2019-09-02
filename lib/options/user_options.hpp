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

    bool pull_profile;
    bool push_profile;

    bool pull_home;
    bool pull_dms;
    bool pull_notifications;
};

enum class user_option {show, accesstoken, username, password, clientsecret, addlist, removelist, home, dms, notifications, newaccount};