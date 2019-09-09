#include "options.hpp"
#include "user_options.hpp"

#include <algorithm>
#include <regex>

#include <print_logger.hpp>

std::optional<parsed_account> parse_account_name(const std::string& name)
{
    const static std::regex account_name{"@?([_a-z0-9]+)@([a-z0-9-]+\\.[a-z0-9-]+(?:\\.[a-z0-9-]+)?)", std::regex::ECMAScript | std::regex::icase};

    std::smatch results;
    if (std::regex_match(name, results, account_name))
    {
        return parsed_account{results[1], results[2]};
    }

    return {};
}

std::variant<const user_options*, const char*> select_account(const std::string_view name)
{
    print_logger<logtype::verbose> pl;

    int matched = 0;
    user_options* candidate = nullptr;

    for (auto& entry : options.accounts)
    {
        // won't have string.starts_with until c++20, so
        // if the name given is a prefix of (or equal to) this entry, it's a candidate
        if (std::equal(name.begin(), name.end(), entry.first.begin()))
        {
            pl << "Matched account" << entry.first << "\n";
            matched++;
            candidate = &entry.second;
        }
    }

    switch (matched)
    {
    case 0:
        return "Couldn't find a match.";
    case 1:
        return candidate;
    default:
        return "Couldn't find an unambiguous match.";
    }
}
