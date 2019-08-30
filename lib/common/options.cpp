#include <string>
#include <vector>

struct global_options {
    bool verbose;

    std::vector<std::string> accounts_to_sync;
    std::vector<std::string> lists_to_sync;
};

struct user_options {
    // including instance name, like name@instance.social
    std::string account_name;
    std::string access_token;

    std::vector<std::string> ids_to_favorite;
    std::vector<std::string> ids_to_boost;
    std::vector<std::string> filenames_to_post;

    bool sync_home;
    bool sync_dms;
    bool sync_notifications;
};