#ifndef _MSYNC_UTIL_HPP_
#define _MSYNC_UTIL_HPP_
#include <string>
#include <string_view>

std::string make_api_url(const std::string_view instance_url, const std::string_view api_route);

#endif
