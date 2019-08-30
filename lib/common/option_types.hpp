#include <string>
#include <vector>

#ifndef __clang__
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

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

struct global_options
{
    bool verbose;
    std::filesystem::path executable_location;
    std::filesystem::path current_working_directory;

    std::vector<user_options> accounts_to_sync;
};