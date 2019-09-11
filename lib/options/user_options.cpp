#include "user_options.hpp"

const std::string* user_options::get_option(user_option opt) const
{
    const auto val = backing.parsed_options.find(USER_OPTION_NAMES[static_cast<int>(opt)]);
    if (val == backing.parsed_options.end())
        return nullptr;

    return &val->second;
}

void user_options::set_option(user_option opt, std::string value)
{
    backing.parsed_options.insert_or_assign(USER_OPTION_NAMES[static_cast<int>(opt)], std::move(value));
}