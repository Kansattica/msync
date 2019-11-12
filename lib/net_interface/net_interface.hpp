#ifndef _MSYNC_NET_INTERFACE_HPP_
#define _MSYNC_NET_INTERFACE_HPP_

#include <string_view>
#include <string>
#include <vector>

#include <filesystem.hpp>

struct net_response
{
	int status_code = 200;
	bool retryable_error = false;
	bool okay = true;
	std::string message;
};

struct status_params
{
	uint64_t idempotency_key;
	std::string body;
	std::string reply_to;
	std::string content_warning;
	std::string visibility;
	std::vector<std::string> attachment_ids;
};

using post_request = net_response (std::string_view url, std::string_view access_token);
using delete_request = net_response (std::string_view url, std::string_view access_token);
using post_new_status = net_response (std::string_view url, std::string_view access_token, const status_params& params);
using upload_attachment = net_response(std::string_view url, std::string_view access_token, const fs::path& file, const std::string& description);

inline constexpr const char* get_error_message(const int status_code, const bool verbose)
{
	switch (status_code)
	{
	case 200:
	case 201:
	case 204:
		return verbose ? "OK\n" : "";

	case 400:
		return "Bad request. This is probably a programming error. Sorry about that.\n";
	case 403:
		return "Forbidden. Is your access token correct?\n";
	case 404:
		return "Not found, is your instance URL and post ID correct?\n";
	case 408:
		return "Request timed out, but may have still gone through.\n";
	case 429:
		return "Rate limited. Will try again later.\n";

	case 500:
	case 501:
	case 502:
	case 503:
	case 504:
		return "Server error. Try again later.\n";

	default:
		return "[unknown response]\n";
	}

}

#endif
