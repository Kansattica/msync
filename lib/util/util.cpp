#include "util.hpp"

#include <regex>

std::string make_api_url(const std::string_view instance_url, const std::string_view api_route)
{
    std::string toreturn{"https://"};
    toreturn.reserve(instance_url.size() + api_route.size() + toreturn.size());
    toreturn.append(instance_url).append(api_route);
    return toreturn;
}

std::optional<parsed_account> parse_account_name(const std::string& name)
{
    const static std::regex account_name{R"(@?([_a-z0-9]+)@(?:https?://)?([a-z0-9-]+\.[a-z0-9-]+(?:\.[a-z0-9-]+)?)[, =/\\?]*$)", std::regex::ECMAScript | std::regex::icase};

    std::smatch results;
    if (std::regex_match(name, results, account_name))
    {
        return parsed_account{results[1], results[2]};
    }

    return {};
}
