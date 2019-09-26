#include "send.hpp"

#include <string>
#include <string_view>
#include <array>
#include <deque>

#include "../options/global_options.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"
#include <print_logger.hpp>

#include <cpr/cpr.h>

using std::string_view;

const std::string statusroute = "/api/v1/statuses/";

const std::array<string_view, 2> favroutepost = { "/favourite", "/unfavourite" };
const std::array<string_view, 2> boostroutepost = { "/reblog", "/unreblog" };

bool simple_post(const std::string& url, const std::string& access_token);

template <const string_view& doroute, const string_view& undoroute, queues toread>
void process_queue(const std::string& account, const std::string& baseurl, int retries);

bool should_undo(string_view& id);

void send(const std::string& account, const std::string& instanceurl, int retries)
{
	print_logger pl;
	pl << "Sending queued favorites for " << account << '\n';

	std::string baseurl = make_api_url(instanceurl, statusroute);
}

void send_all(int retries)
{
	for (auto& user : options.accounts)
	{
		send(user.first, *user.second.get_option(user_option::instance_url), retries);
	}

}

std::string paramaterize_url(const string_view before, const string_view middle, const string_view after)
{
	std::string toreturn{ before };
	toreturn.reserve(toreturn.size() + middle.size() + after.size());
	return toreturn.append(middle).append(after);
}

template <const string_view& doroute, const string_view& undoroute, queues toread>
void process_queue(const std::string& account, const std::string& baseurl, int retries)
{
	auto queuefile = get(toread, account);

	std::deque<std::string> failedids;

	while (!queuefile.queued.empty())
	{
		std::string_view id = queuefile.queued.front();

		bool undo = should_undo(id);

		std::string requesturl = paramaterize_url(baseurl, id, undo ? undoroute : doroute);

	}

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
