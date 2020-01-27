#ifndef _SYNC_HELPERS_HPP_
#define _SYNC_HELPERS_HPP_

#include <string_view>
#include <optional>
#include <chrono>

#include <filesystem.hpp>

#include "../net_interface/net_interface.hpp"
#include "read_response.hpp"

constexpr std::string_view status_route{ "/api/v1/statuses/" };
constexpr std::string_view media_route{ "/api/v1/media" };

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

template <typename Stream>
void print_statistics(Stream& os, long long time_ms, unsigned int tries)
{
	os << " (" << time_ms << " ms";
	if (tries != 1)
		os << tries << " attempts";
	os << ")\n";
}

struct request_response
{
	bool success;
	std::string message;
	unsigned int tries;
	long long time_ms;
};

template <typename make_request, typename Stream>
request_response request_with_retries(make_request req, unsigned int retries, Stream& os)
{
	const auto start_time = std::chrono::steady_clock::now();
	for (unsigned int i = 0; i < retries; i++)
	{
		net_response response = req();

		const auto end_time = std::chrono::steady_clock::now();

		// later, handle what happens if we get rate limited

		if (response.retryable_error)
		{
			// should retry
			continue;
		}

		// some other error, assume unrecoverable
		if (!response.okay)
		{
			os << '\n' << response.status_code << ": " << get_error_message(response.status_code, false);

			auto parsed_error = read_error(response.message);
			if (!parsed_error.empty())
				response.message = std::move(parsed_error);
			os << "Remote server returned an error: " << response.message;
			return request_response{ false, std::move(response.message), i + 1, std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() };
		}

		// must be 200, OK response
		return request_response{ true, std::move(response.message), i + 1, std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() };
	}

	const auto end_time = std::chrono::steady_clock::now();

	os << "Maximum retries reached.\n";
	return request_response{ false,  "Maximum retries reached.", retries, std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() };
}
#endif
