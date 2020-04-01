#ifndef _SEND_HELPERS_HPP_
#define _SEND_HELPERS_HPP_

#include "../queue/queue_list.hpp"
#include <print_logger.hpp>

#include "../util/util.hpp"

#include <array>
#include <string_view>
#include <map>
#include <utility>
#include <string>

// make sure the ones with an argument after them have a trailing slash!
constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_LOOKUP = {
 "/api/v1/statuses/favourite/", 
 "/api/v1/statuses/unfavourite/", 
 "/api/v1/statuses/reblog/",
 "/api/v1/statuses/unreblog/", 
 "/api/v1/statuses", 
 "/api/v1/statuses/",
 "/api/v1/media"
};

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

const std::string& make_lookup_url(std::string_view instance_url, api_route method)
{
	static std::map<std::pair<api_route, std::string_view>, std::string> created_urls;

	auto to_lookup = std::make_pair(method, instance_url);

	const auto found_it = created_urls.find(to_lookup);

	if (found_it != created_urls.end())
	{
		return found_it->second;
	}

	const auto inserted = created_urls.insert({ std::move(to_lookup), make_api_url(instance_url, ROUTE_LOOKUP[static_cast<uint8_t>(method)]) });
	return inserted.first->second;
}


#endif
