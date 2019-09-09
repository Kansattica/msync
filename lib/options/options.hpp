#ifndef _OPTIONS_H_
#define _OPTIONS_H_
#include "global_options.hpp"

#include <optional>
#include <string>
#include <string_view>

struct parsed_account
{
    std::string username;
    std::string instance;
};

std::optional<parsed_account> parse_account_name(const std::string& name);
const user_options* select_account(const std::string_view name);
#endif