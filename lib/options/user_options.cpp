#include "user_options.hpp"

#include <array>
#include <cassert>
#include <utility>

const std::string* user_options::get_option(user_option toget) const
{
    const auto val = backing.parsed.find(USER_OPTION_NAMES[static_cast<size_t>(toget)]);
    if (val == backing.parsed.end())
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
    auto option = static_cast<size_t>(toget);
    const auto val = backing.parsed.find(USER_OPTION_NAMES[option]);
    if (val == backing.parsed.end())
        return sync_setting_defaults[option - static_cast<size_t>(user_option::pull_home)];
    return parse_enum<sync_settings>(val->second[0]);
}

void user_options::set_option(user_option opt, std::string value)
{
    backing.parsed.insert_or_assign(USER_OPTION_NAMES[static_cast<size_t>(opt)], std::move(value));
}

void user_options::set_option(user_option opt, list_operations value)
{
    backing.parsed.insert_or_assign(USER_OPTION_NAMES[static_cast<size_t>(opt)], LIST_OPERATION_NAMES[static_cast<size_t>(value)]);
}

void user_options::set_option(user_option opt, sync_settings value)
{
    backing.parsed.insert_or_assign(USER_OPTION_NAMES[static_cast<size_t>(opt)], SYNC_SETTING_NAMES[static_cast<size_t>(value)]);
}