#ifndef SYNC_HELPERS_HPP
#define SYNC_HELPERS_HPP

#include <string_view>
#include <optional>
#include <chrono>
#include <thread>
#include <algorithm>

#include <filesystem.hpp>

#include "../netinterface/net_interface.hpp"
#include "../util/util.hpp"

#include "read_response.hpp"

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
		truncated = true;
		toprint.remove_suffix(toprint.size() - max_length);
	}

	const auto newline_idx = toprint.find('\n');
	if (newline_idx != std::string::npos)
	{
		// only print the dots if something other than whitespace got chopped off
		// or if it's already been truncated due to length
		truncated = truncated || (toprint.find_first_not_of(" \r\n\t", newline_idx) != std::string::npos);
		toprint.remove_suffix(toprint.size() - newline_idx);
	}

	while (!toprint.empty() && toprint.back() == ' ')
		toprint.remove_suffix(1);

	str << toprint;
	if (truncated)
		str << "...";
	else if (toprint.empty())
		str << "(no body)";
}

template <typename Stream>
void print_statistics(Stream& os, long long time_ms, unsigned int tries)
{
	os << " (" << time_ms << " ms";
	if (tries != 1) { os << ", " << tries << " attempts"; }
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
	// Basically, before this is called, a URL is printed, and console IO buffers until it sees a newline.
	// I want people to see the URL for the request that's happening, while it's happening.
	os.flush();
	const auto start_time = std::chrono::steady_clock::now();
	bool rate_limit_waited = false;
	for (unsigned int i = 0; i < retries; i++)
	{
		net_response response = req();

		const auto end_time = std::chrono::steady_clock::now();

		if (response.retryable_error)
		{
			if (response.status_code == 429)
			{
				const auto resets_at = parse_ISO8601_timestamp(response.message);

				os << '\n';
				do
				{
					// always wait a minimum of ten seconds
					const auto estimated_wait = std::max(std::chrono::duration_cast<std::chrono::seconds>(resets_at - std::chrono::system_clock::now()), std::chrono::seconds{10});
					os << "429: Rate limited.";

					os << " Waiting ";
					const bool print_minutes = estimated_wait >= std::chrono::minutes(1);
					if (print_minutes)
					{
						const auto mins = std::chrono::duration_cast<std::chrono::minutes>(estimated_wait).count();
						os << mins << pluralize(mins, " minute", " minutes");
					}
					const auto seconds_part = estimated_wait.count() % 60;

					if (seconds_part == 0)
						os << ".           ";
					else
						os << (print_minutes ? ", " : "") << seconds_part << pluralize(seconds_part, " second.", " seconds.");
					os << "                \r"; // make sure line is cleared and carriage return for live updates.
					os.flush(); // tell the user what they're waiting for
					std::this_thread::sleep_for(std::min(estimated_wait, std::chrono::seconds{5}));
				} while (std::chrono::system_clock::now() < resets_at);
				os << "                                 \rFinished waiting for rate limit to reset. Retrying.";
				rate_limit_waited = true;
			}
			// should retry
			continue;
		}

		// some other error, assume unrecoverable
		if (!response.okay)
		{
			os << '\n' << response.status_code << ": " << get_error_message(response.status_code, rate_limit_waited);

			auto parsed_error = read_error(response.message);
			if (!parsed_error.empty())
				response.message = std::move(parsed_error);
			os << "Remote server returned an error: " << response.message;
		}

		// must be 200, OK response
		return request_response{ response.okay, std::move(response.message), i + 1, std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() };
	}

	const auto end_time = std::chrono::steady_clock::now();

	os << " Error: Maximum retries reached.";
	return request_response{ false,  "Maximum retries reached.", retries, std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() };
}
#endif
