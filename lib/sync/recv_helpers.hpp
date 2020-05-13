#ifndef RECV_HELPERS_HPP
#define RECV_HELPERS_HPP

#include <string_view>
#include <string>

#include "../options/user_options.hpp"

#include "../constants/constants.hpp"

#include "read_response.hpp"

enum class to_get { notifications, home, dms, lists };

struct recv_parameters { user_option last_id_setting; user_option sync_setting; std::string_view route; const CONSTANT_PATH_TYPE& filename; };

constexpr std::string_view home_route{ "/api/v1/timelines/home" };
constexpr std::string_view notifications_route{ "/api/v1/notifications" };

template <to_get timeline>
CONSTEXPR_IF_NOT_BOOST recv_parameters get_parameters()
{
	if CONSTEXPR_IF_NOT_BOOST (timeline == to_get::notifications)
	{
		return { user_option::last_notification_id, user_option::pull_notifications, notifications_route, Notifications_Filename };
	}

	if CONSTEXPR_IF_NOT_BOOST (timeline == to_get::home)
	{
		return { user_option::last_home_id, user_option::pull_home, home_route, Home_Timeline_Filename };
	}
}

template <typename entity>
std::string highest_id(const std::vector<entity>& chunk)
{
	return chunk.front().id;
}

template <typename entity>
std::string lowest_id(const std::vector<entity>& chunk)
{
	return chunk.back().id;
}

template <typename entity>
bool contains_id(const std::vector<entity>& chunk, std::string_view id)
{
	if (id.empty()) { return false; }
	return std::any_of(chunk.begin(), chunk.end(), [&id](const entity& elem) { return elem.id == id; });
}

template <typename entity>
std::vector<entity> deserialize(const std::string& json);

template <>
std::vector<mastodon_notification> deserialize(const std::string& json)
{
	return read_notifications(json);
}

template <>
std::vector<mastodon_status> deserialize(const std::string& json)
{
	return read_statuses(json);
}

std::string_view get_or_empty(const std::string* str)
{
	if (str == nullptr)
		return "";
	return *str;
}

unsigned int clamp_or_default(unsigned int input, unsigned int maxdefault)
{
	if (input == 0 || input > maxdefault) { return maxdefault; }
	return input;
}

template <typename Stream>
void print_api_call(std::string_view url, unsigned int limit, const timeline_params& params, Stream& os)
{
	os << "GET " << url << "?limit=" << limit;

	if (!params.max_id.empty())
		os << "&max_id=" << params.max_id;

	if (!params.min_id.empty())
		os << "&min_id=" << params.min_id;

	if (!params.since_id.empty())
		os << "&since_id=" << params.since_id;

	if (params.exclude_notifs != nullptr && !params.exclude_notifs->empty())
	{
		for (const auto& type : *params.exclude_notifs)
		{
			os << "&exclude_types[]=" << type;
		}
	}

	os.flush(); //print this now, don't wait for a newline
	// no newline, print_statistics will do that
}

std::vector<std::string_view> make_excludes(const user_options& account)
{
	static constexpr std::array<std::pair<user_option, std::string_view>, 5> option_name_pairs =
	{
		std::make_pair(user_option::exclude_follows, "follow"),
		std::make_pair(user_option::exclude_favs, "favourite"),
		std::make_pair(user_option::exclude_boosts, "reblog"),
		std::make_pair(user_option::exclude_mentions, "mention"),
		std::make_pair(user_option::exclude_polls, "poll")
	};

	std::vector<std::string_view> toreturn;

	for (const auto& pair : option_name_pairs)
	{
		if (account.get_bool_option(pair.first))
			toreturn.push_back(pair.second);
	}

	return toreturn;
}
#endif
