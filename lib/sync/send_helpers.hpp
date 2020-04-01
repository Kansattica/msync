#ifndef _SEND_HELPERS_HPP_
#define _SEND_HELPERS_HPP_

#include "../queue/queue_list.hpp"

#include <array>
#include <string_view>

constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_LOOKUP = {
 "/favourite", "/unfavourite", "/reblog", "/unreblog", "", ""
};

#endif
