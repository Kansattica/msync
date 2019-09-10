#include "options.hpp"

#include <regex>

std::optional<parsed_account> parse_account_name(const std::string& name)
{
    const static std::regex account_name{"@?([_a-z0-9]+)@(?:https?://)?([a-z0-9-]+\\.[a-z0-9-]+(?:\\.[a-z0-9-]+)?)[, =/\\\\?]*$", std::regex::ECMAScript | std::regex::icase};

    std::smatch results;
    if (std::regex_match(name, results, account_name))
    {
        return parsed_account{results[1], results[2]};
    }

    return {};
}