#include "send.hpp"

#include "../net/net.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <deque>

#include "../options/global_options.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"

#include <print_logger.hpp>


using std::string_view;

constexpr string_view statusroute = "/api/v1/statuses/";

constexpr std::pair<string_view, string_view> favroutepost = { "/favourite", "/unfavourite" };
constexpr std::pair<string_view, string_view> boostroutepost = { "/reblog", "/unreblog" };

template <queues toread, typename network>
void process_queue(const string_view account, const string_view baseurl, const network& net, int retries);

bool should_undo(string_view& id);

template <typename network>
void send(const std::string& account, const std::string& instanceurl, const std::string& access_token, int retries)
{
	if (retries < 1)
	{
		pl() << "Number of retries must be positive (got " << retries << "). Resetting to 3.\n";
		retries = 3;
	}

	std::string baseurl = make_api_url(instanceurl, statusroute);

	network net;
	net.access_token = access_token;

	pl() << "Sending queued favorites for " << account << '\n';
	process_queue<queues::fav>(account, baseurl, net, retries);

	pl() << "Sending queued boosts for " << account << '\n';
	process_queue<queues::boost>(account, baseurl, net, retries);
}

template <typename network>
void send_all(int retries)
{
	for (auto& user : options.accounts)
	{
		send<network>(user.first, *user.second.get_option(user_option::instance_url), *user.second.get_option(user_option::access_token), retries);
	}
}

std::string paramaterize_url(const string_view before, const string_view middle, const string_view after)
{
	std::string toreturn{ before };
	toreturn.reserve(toreturn.size() + middle.size() + after.size());
	return toreturn.append(middle).append(after);
}

template <typename network>
bool post_with_retries(const string_view requesturl, const network& net, int retries)
{
	for (int i = 0; i < retries; i++)
	{
		auto response = net.simple_post(requesturl);

		pl() << net.get_error_message(response.status_code, verbose_logs);

		// later, handle what happens if we get rate limited

		if (response.retryable_error)
		{
			// should retry
			continue;
		}

		// some other error, assume unrecoverable
		if (!response.okay)
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

template <queues toread, typename network>
void process_queue(const string_view account, const string_view baseurl, const network& net, int retries)
{
	auto queuefile = get(toread, account);

	std::deque<std::string> failedids;

	while (!queuefile.parsed.empty())
	{
		string_view id = queuefile.parsed.front();

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

bool should_undo(string_view& id)
{
	if (id.back() == '-')
	{
		id.remove_suffix(1);
		return true;
	}

	return false;
}
