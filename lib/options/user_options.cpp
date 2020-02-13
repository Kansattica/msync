#include "user_options.hpp"

#include <array>
#include <cassert>
#include <utility>
#include <string_view>

#include "../exception/msync_exception.hpp"

const std::string* user_options::try_get_option(user_option toget) const
{
    const auto val = backing.parsed.find(USER_OPTION_NAMES[static_cast<size_t>(toget)]);
    if (val == backing.parsed.end())
        return nullptr;

    return &val->second;
}

std::string make_error_message(const std::string_view& option_name)
{
    return std::string{ "msync cannot continue because the following option was not set: " }.append(option_name);
}

const std::string& user_options::get_option(user_option toget) const
{
    const auto& option_name = USER_OPTION_NAMES[static_cast<size_t>(toget)];
    const auto val = backing.parsed.find(option_name);
    if (val == backing.parsed.end())
        throw msync_exception(make_error_message(option_name));

    return val->second;
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
    const auto option = static_cast<size_t>(toget);
    const auto val = backing.parsed.find(USER_OPTION_NAMES[option]);
    if (val == backing.parsed.end())
        return sync_setting_defaults[option - static_cast<size_t>(user_option::pull_home)];
    return parse_enum<sync_settings>(val->second[0]);
}

bool user_options::get_bool_option(user_option toget) const
{
    const auto val = backing.parsed.find(USER_OPTION_NAMES[static_cast<size_t>(toget)]);
    if (val == backing.parsed.end() || val->second.empty())
        return false;

    const auto firstchar = val->second[0];

    // true or yes are truthy, everything else is falsy
    return firstchar == 't' || firstchar == 'T' || firstchar == 'y' || firstchar == 'Y';
}

void user_options::set_option(user_option opt, std::string value)
{
	backing.parsed.insert_or_assign(std::string{ USER_OPTION_NAMES[static_cast<size_t>(opt)] }, std::move(value));
}

void user_options::set_option(user_option opt, list_operations value)
{
	backing.parsed.insert_or_assign(std::string{ USER_OPTION_NAMES[static_cast<size_t>(opt)] }, std::string{ LIST_OPERATION_NAMES[static_cast<size_t>(value)] });
}

void user_options::set_option(user_option opt, sync_settings value)
{
	backing.parsed.insert_or_assign(std::string{ USER_OPTION_NAMES[static_cast<size_t>(opt)] }, std::string{ SYNC_SETTING_NAMES[static_cast<size_t>(value)] });
}


// I have to call it set_bool_option or else c++ will try to use this overload with char* string literals
void user_options::set_bool_option(user_option opt, bool value)
{
	// this should save a strlen call at runtime
	static constexpr std::string_view true_sv = "true";
	static constexpr std::string_view false_sv = "false";
	backing.parsed.insert_or_assign(std::string{ USER_OPTION_NAMES[static_cast<size_t>(opt)] }, std::string{ value ? true_sv : false_sv });
}
