#ifndef _MSYNC_SEND_HPP_
#define _MSYNC_SEND_HPP_

#include <print_logger.hpp>
#include <string>
#include <string_view>

#include <utility>
#include <deque>

#include "../net_interface/net_interface.hpp"
#include "../options/user_options.hpp"
#include "../options/global_options.hpp"
#include "../queue/queues.hpp"
#include "../util/util.hpp"

template <post_request post>
struct send_posts
{
public:
	int retries = 3;

	void send_all()
	{
		for (auto& user : options.accounts)
		{
			send(user.first, *user.second.get_option(user_option::instance_url), *user.second.get_option(user_option::access_token));
		}
	}

	void send(const std::string& account, const std::string& instanceurl, const std::string_view access_token)
	{
		if (retries < 1)
		{
			pl() << "Number of retries must be positive (got " << retries << "). Resetting to 3.\n";
			retries = 3;
		}

		std::string baseurl = make_api_url(instanceurl, status_route());

		this->access_token = access_token;
		pl() << "Sending queued favorites for " << account << '\n';
		process_queue<queues::fav>(account, baseurl);

		pl() << "Sending queued boosts for " << account << '\n';
		process_queue<queues::boost>(account, baseurl);
	}

private:
	std::string_view access_token;

	constexpr const std::string_view status_route() const { return "/api/v1/statuses/"; }

	constexpr const std::pair<std::string_view, std::string_view> favroutepost() const { return { "/favourite", "/unfavourite" }; }
	constexpr const std::pair<std::string_view, std::string_view> boostroutepost() const { return { "/reblog", "/unreblog" }; }

	template <queues toread>
	void process_queue(const std::string_view account, const std::string_view baseurl) const
	{
		auto queuefile = get(toread, account);

		std::deque<std::string> failedids;

		while (!queuefile.parsed.empty())
		{
			std::string_view id = queuefile.parsed.front();

			bool undo = should_undo(id);

			std::string requesturl = paramaterize_url(baseurl, id, undo ? route<toread, false>() : route<toread, true>());

			if (post_with_retries(requesturl) == false)
			{
				failedids.emplace_back(id);
			}

			// remove ID from this queue
			queuefile.parsed.pop_front();
		}

		queuefile.parsed = failedids;
	}

	template <queues tosend, bool create>
	constexpr std::string_view route() const
	{
		std::pair<std::string_view, std::string_view> toreturn;
		if constexpr (tosend == queues::fav)
		{
			toreturn = favroutepost();
		}

		if constexpr (tosend == queues::boost)
		{
			toreturn = boostroutepost();
		}

		return std::get<create ? 0 : 1>(toreturn);
	}

	bool post_with_retries(const std::string_view requesturl) const
	{
		for (int i = 0; i < retries; i++)
		{
			auto response = post(requesturl, access_token);

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
				return false;
			}

			// must be 200, OK response
			return true;
		}
		return false;
	}

	constexpr bool send_posts::should_undo(std::string_view& id) const
	{
		if (id.back() == '-')
		{
			id.remove_suffix(1);
			return true;
		}

		return false;
	}

	std::string paramaterize_url(const std::string_view before, const std::string_view middle, const std::string_view after) const
	{
		std::string toreturn{ before };
		toreturn.reserve(toreturn.size() + middle.size() + after.size());
		return toreturn.append(middle).append(after);
	}
};

#endif