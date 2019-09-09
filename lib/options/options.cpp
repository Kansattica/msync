#include "options.hpp"
#include "user_options.hpp"

#include <algorithm>

#include <printlogger.hpp>

global_options options;

std::variant<const user_options*, const std::string> select_account(const std::string& name)
{
    PrintLogger<logtype::verbose> pl;

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
