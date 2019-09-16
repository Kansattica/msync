#include "user_options.hpp"

#include <array>
#include <cassert>

const std::string* user_options::get_option(user_option toget) const
{
    const auto val = backing.backed.find(USER_OPTION_NAMES[static_cast<int>(toget)]);
    if (val == backing.backed.end())
        return nullptr;

    return &val->second;
}

std::array<sync_settings, 3> sync_setting_defaults = {
    sync_settings::newest_first, //pull_home
    sync_settings::oldest_first, //pull_dms
    sync_settings::oldest_first  //pull_notifications
};

sync_settings user_options::get_sync_option(user_option toget) const
{
    //only these guys have sync options
    assert(toget == user_option::pull_home || toget == user_option::pull_dms || toget == user_option::pull_notifications);
    int option = static_cast<int>(toget);
    const auto val = backing.backed.find(USER_OPTION_NAMES[option]);
    if (val == backing.backed.end())
        return sync_setting_defaults[option - static_cast<int>(user_option::pull_home)];
    return parse_enum<sync_settings>(val->second[0]);
}

void user_options::set_option(user_option opt, std::string value)
{
    backing.backed.insert_or_assign(USER_OPTION_NAMES[static_cast<int>(opt)], std::move(value));
}

void user_options::set_option(user_option opt, list_operations value)
{
    backing.backed.insert_or_assign(USER_OPTION_NAMES[static_cast<int>(opt)], LIST_OPERATION_NAMES[static_cast<int>(value)]);
}

void user_options::set_option(user_option opt, sync_settings value)
{
    backing.backed.insert_or_assign(USER_OPTION_NAMES[static_cast<int>(opt)], SYNC_SETTING_NAMES[static_cast<int>(value)]);
}