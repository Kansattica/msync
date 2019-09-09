#include "options.hpp"
#include "user_options.hpp"

#include <algorithm>
#include <vector>

#include <printlogger.hpp>

global_options options;

std::variant<const user_options* const, const std::string> select_account(const std::string& name)
{
    PrintLogger<logtype::verbose> pl;

    std::vector<user_options*> candidates;

    for (auto& entry : options.accounts)
    {
        // won't have string.starts_with until c++20, so
        // if the name given is a prefix of (or equal // to) this entry, it's a candidate
        if (std::equal(name.begin(), name.end(), entry.first.begin()))
        {
            pl << "Matched account" << entry.first << "\n";
            candidates.push_back(&(entry.second));
        }
    }

    if (candidates.size() == 1) //if we found an unambiguous match
    {
        return candidates[0];
    }

    if (candidates.size() > 1)
    {
        return "Couldn't find an unambiguous match.";
    }

    return "Couldn't find a match.";
}
