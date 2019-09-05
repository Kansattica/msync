#include "user_options.hpp"

#include <array>

const auto optionsToKeys =
    std::array<std::string, static_cast<int>(user_option::notifications) + 1>(
        {"account_name",
         "instance_url",
         "access_token",
         "username",
         "password",
         "client_secret",
         "pull_home",
         "pull_dms",
         "pull_notifications"});

std::optional<std::string> user_options::get_option(user_option opt)
{
    auto val = backing.parsed_options.find(optionsToKeys[static_cast<int>(opt)]);
    if (val == backing.parsed_options.end())
        return {};

    return val->second;
}

void user_options::set_option(user_option opt, std::string value)
{
    backing.parsed_options[optionsToKeys[static_cast<int>(opt)]] = value;
}