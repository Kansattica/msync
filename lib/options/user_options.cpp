#include "user_options.hpp"

#include <array>

using namespace std::string_literals;

const auto optionsToKeys =
    std::array<const std::string,
               static_cast<int>(user_option::notifications) + 1>(
        {"account_name"s, "instance_url"s, "access_token"s, "username"s, "password"s,
         "client_secret"s, "pull_home"s, "pull_dms"s, "pull_notifications"s});

const std::string* user_options::get_option(user_option opt) const
{
    const auto val =
        backing.parsed_options.find(optionsToKeys[static_cast<int>(opt)]);
    if (val == backing.parsed_options.end())
        return nullptr;

    return &val->second;
}

void user_options::set_option(user_option opt, std::string&& value)
{
    backing.parsed_options.insert_or_assign(optionsToKeys[static_cast<int>(opt)],
                                            std::forward<std::string>(value));
}