#include "send.hpp"

#include "error_code_messages.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <deque>

#include "../options/global_options.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"

#include <print_logger.hpp>

#include <cpr/cpr.h>

using std::string_view;

const std::string statusroute = "/api/v1/statuses/";

constexpr std::pair<string_view, string_view> favroutepost = { "/favourite", "/unfavourite" };
constexpr std::pair<string_view, string_view> boostroutepost = { "/reblog", "/unreblog" };

cpr::Response simple_post(const std::string& url, const std::string& access_token);

template <queues toread>
void process_queue(const std::string& account, const std::string& baseurl, const std::string& access_token, int retries);

bool should_undo(string_view& id);

void send(const std::string& account, const std::string& instanceurl, const std::string& access_token, int retries)
{
	print_logger pl;

	if (retries < 1)
	{
		pl << "Number of retries must be positive (got " << retries << "). Resetting to 3.\n";
		retries = 3;
	}

	std::string baseurl = make_api_url(instanceurl, statusroute);

	pl << "Sending queued favorites for " << account << '\n';
	process_queue<queues::fav>(account, baseurl, access_token, retries);

	pl << "Sending queued boosts for " << account << '\n';
	process_queue<queues::boost>(account, baseurl, access_token, retries);
}

void send_all(int retries)
{
	for (auto& user : options.accounts)
	{
		send(user.first, *user.second.get_option(user_option::instance_url), *user.second.get_option(user_option::access_token), retries);
	}

}

std::string paramaterize_url(const string_view before, const string_view middle, const string_view after)
{
	std::string toreturn{ before };
	toreturn.reserve(toreturn.size() + middle.size() + after.size());
	return toreturn.append(middle).append(after);
}

bool post_with_retries(const std::string& requesturl, const std::string& access_token, int retries)
{
	for (int i = 0; i < retries; i++)
	{
		auto response = simple_post(requesturl, access_token);

		if (response.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT || (response.status_code >= 500 && response.status_code < 600))
		{
			// should retry
			continue;
		}

		// some other error, assume unrecoverable
		if (response.error)
		{
			return false;
		}

		// must be 200, OK response
		return true;
	}
	return false;
}

template <queues tosend, bool create>
constexpr string_view route()
{
	std::pair<string_view, string_view> toreturn;
	if constexpr (tosend == queues::fav)
	{
		toreturn = favroutepost;
	}

	if constexpr (tosend == queues::boost)
	{
		toreturn = boostroutepost;
	}

	return std::get<create ? 0 : 1>(toreturn);

}

template <queues toread>
void process_queue(const std::string& account, const std::string& baseurl, const std::string& access_token, int retries)
{
	auto queuefile = get(toread, account);

	std::deque<std::string> failedids;

	while (!queuefile.parsed.empty())
	{
		std::string_view id = queuefile.parsed.front();

		bool undo = should_undo(id);

		std::string requesturl = paramaterize_url(baseurl, id, undo ? route<toread, false>() : route<toread, true>());

		if (post_with_retries(requesturl, access_token, retries) == false)
		{
			failedids.emplace_back(id);
		}

		// remove ID from this queue
		queuefile.parsed.pop_front();
	}

	queuefile.parsed = failedids;
}


cpr::Response simple_post(const std::string& url, const std::string& access_token)
{
	auto response = cpr::Post(cpr::Url{ url }, cpr::Authentication{ "Bearer", access_token }, cpr::Header{ {"Idempotency-Key", url} });

	print_logger pl;

	if (response.error)
	{
		pl << response.error.message << '\n';
	}

	pl << get_error_message(response.status_code, verbose_logs);

	return response;
}


bool should_undo(string_view& id)
{
	if (id.back() == '-')
	{
		id.remove_suffix(1);
		return true;
	}

	return false;
}
