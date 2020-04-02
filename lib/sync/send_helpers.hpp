#ifndef _SEND_HELPERS_HPP_
#define _SEND_HELPERS_HPP_

#include "../queue/queue_list.hpp"
#include <print_logger.hpp>

#include "../util/util.hpp"

#include <array>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <string>

constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_LOOKUP = {
 "/favourite", 
 "/unfavourite", 
 "/reblog",
 "/unreblog", 
 "", 
 ""
};

constexpr std::string_view STATUS_ROUTE{ "/api/v1/statuses/" };
constexpr std::string_view MEDIA_ROUTE{ "/api/v1/media" };

template <typename make_request>
bool simple_call(make_request& method, const char* method_name, unsigned int retries, const std::string& url, std::string_view access_token)
{
	pl() << method_name << ' ' << url;
	const auto response = request_with_retries([&]() { return method(url, access_token); }, retries, pl());
	if (response.success)
		pl() << " OK";
	print_statistics(pl(), response.time_ms, response.tries);
	return response.success;
}

#endif
