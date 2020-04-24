#ifndef SEND_HELPERS_HPP
#define SEND_HELPERS_HPP

#include "../queue/queue_list.hpp"
#include <print_logger.hpp>

#include "../util/util.hpp"

#include <array>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <string>

#include "read_response.hpp"

#include "../netinterface/net_interface.hpp"

constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_LOOKUP = {
 "/favourite", 
 "/unfavourite", 
 "/reblog",
 "/unreblog", 
 "", 
 ""
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

std::string paramaterize_url(std::string_view before, std::string_view middle, std::string_view after);

void store_thread_id(std::string msync_id, std::string remote_server_id);

struct attachment
{
	fs::path file;
	std::string description;
};

struct file_status_params : public status_params
{
	std::vector<attachment> attachments;
	std::string reply_id;
	bool okay = true;
};

file_status_params read_params(const fs::path& path);

#endif
