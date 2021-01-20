#ifndef SEND_HELPERS_HPP
#define SEND_HELPERS_HPP

#include "../queue/queue_list.hpp"
#include <print_logger.hpp>
#include <filesystem.hpp>

#include "../util/util.hpp"

#include "../constants/constants.hpp"

#include <array>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <string>

#include "read_response.hpp"

#include "sync_helpers.hpp"

#include "../netinterface/net_interface.hpp"

constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_LOOKUP = {
 "/favourite", 
 "/unfavourite", 
 "/reblog",
 "/unreblog", 
 "/bookmark",
 "/unbookmark", 
 "", 
 "",
 "/context"
};


template <typename make_request>
request_response simple_call(make_request& method, const char* method_name, unsigned int retries, const std::string& url, std::string_view access_token)
{
	pl() << method_name << ' ' << url;
	const auto response = request_with_retries([&]() { return method(url, access_token); }, retries, pl());
	if (response.success)
		pl() << " OK";
	print_statistics(pl(), response.time_ms, response.tries);
	return response;
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

void write_posts(const mastodon_context& context, const mastodon_status& status, const fs::path& path);

template <typename make_request>
bool get_and_write(make_request& method, const fs::path& user_account_dir, unsigned int retries, const std::string& status_url, const std::string& post_id, std::string_view access_token)
{
	auto adapted_get = [&method](const auto& request_url, const auto& access_token) { return method(request_url, access_token, timeline_params{}, 0); };
	// GET https://instance.url/api/v1/statuses/post_id
	auto request_url = status_url + post_id;
	const auto status_response = simple_call(adapted_get, "GET", retries, request_url, access_token);
	if (!status_response.success) { return false; }

	// this might have to become more general, like what's done in recv.hpp, but it's fine for now.
	// basically, we get the message they want context for, then we get the context around it
	// because the context call doesn't include the message itself.

	// GET https://instance.url/api/v1/statuses/post_id/context
	request_url += "/context";
	const auto context_response = simple_call(adapted_get, "GET", retries, request_url, access_token);
	if (!context_response.success) { return false; }

	// build up the target file location to minimize the number of intermediate strings that get thrown away
	auto post_file = user_account_dir / Thread_Directory;
	post_file /= post_id;
	post_file += ".list";

	write_posts(read_context(context_response.message), read_status(status_response.message), post_file);

	return true;
}

#endif
