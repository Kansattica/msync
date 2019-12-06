#ifndef _SYNC_HELPERS_HPP_
#define _SYNC_HELPERS_HPP_

#include <string_view>
#include <optional>

#include <filesystem.hpp>

#include "../net_interface/net_interface.hpp"

constexpr std::string_view status_route{ "/api/v1/statuses/" };
constexpr std::string_view media_route{ "/api/v1/media" };
constexpr std::string_view home_route{ "/api/v1/timelines/home" };
constexpr std::string_view notifications_route{ "/api/v1/notifications" };

constexpr std::pair<std::string_view, std::string_view> favroutepost{ "/favourite", "/unfavourite" };
constexpr std::pair<std::string_view, std::string_view> boostroutepost{ "/reblog", "/unreblog" };

bool should_undo(std::string_view& id);

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

template <typename message_type, typename stream_output>
unsigned int set_default(unsigned int value, unsigned int default_value, const message_type& message, stream_output& out)
{
	if (value == 0)
	{
		out << message;
		return default_value;
	}

	return value;
}

template <typename Stream>
void print_truncated_string(std::string_view toprint, Stream& str)
{
	static constexpr int max_length = 60;
	bool truncated = false;
	if (toprint.size() > max_length)
	{
		toprint.remove_suffix(toprint.size() - max_length);
		truncated = true;
	}

	if (!toprint.empty() && toprint.back() == '\n')
		toprint.remove_suffix(1);

	str << toprint;
	if (truncated)
		str << "...";
}

template <typename make_request>
std::pair<bool, std::string> request_with_retries(make_request req, unsigned int retries)
{
	for (unsigned int i = 0; i < retries; i++)
	{
		net_response response = req();

		pl() << get_error_message(response.status_code, verbose_logs);

		// later, handle what happens if we get rate limited

		if (response.retryable_error)
		{
			// should retry
			continue;
		}

		// some other error, assume unrecoverable
		if (!response.okay)
		{
			auto parsed_error = read_error(response.message);
			if (!parsed_error.empty())
				response.message = std::move(parsed_error);
			pl() << response.message << '\n';
			return std::make_pair(false, std::move(response.message));
		}

		// must be 200, OK response
		return std::make_pair(true, std::move(response.message));
	}
	return std::make_pair(false, "Maximum retries reached.");
}
#endif
