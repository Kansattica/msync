#include "util.hpp"

std::string make_api_url(const std::string_view instance_url, const std::string_view api_route)
{
    std::string toreturn{"https://"};
    toreturn.reserve(instance_url.size() + api_route.size() + toreturn.size());
    toreturn.append(instance_url).append(api_route);
    return toreturn;
}