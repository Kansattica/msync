#ifndef _RECV_HELPERS_HPP_
#define _RECV_HELPERS_HPP_

#include <type_traits>
#include <string_view>
#include <string>

#include "../constants/constants.hpp"

#include "read_response.hpp"

enum class to_get { notifications, home, dms, lists };

struct recv_parameters { user_option last_id_setting; user_option sync_setting; std::string_view route; std::string_view filename; };

constexpr std::string_view home_route{ "/api/v1/timelines/home" };
constexpr std::string_view notifications_route{ "/api/v1/notifications" };
constexpr std::string_view dm_route{ "/api/v1/conversations" };

template <to_get timeline>
constexpr recv_parameters get_parameters()
{
	if constexpr (timeline == to_get::notifications)
	{
		return { user_option::last_notification_id, user_option::pull_notifications, notifications_route, Notifications_Filename };
	}

	if constexpr (timeline == to_get::home)
	{
		return { user_option::last_home_id, user_option::pull_home, home_route, Home_Timeline_Filename };
	}

	if constexpr (timeline == to_get::dms)
	{
		return { user_option::last_dm_id, user_option::last_dm_id, dm_route, Direct_Messages_Filename };
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
std::vector<entity> deserialize(const std::string& json)
{
	if constexpr (std::is_same<entity, mastodon_notification>::value)
	{
		return read_notifications(json);
	}

	if constexpr (std::is_same<entity, mastodon_status>::value)
	{
		return read_statuses(json);
	}
}

std::string_view get_or_empty(const std::string* str)
{
	if (str == nullptr)
		return "";
	return *str;
}

template <typename Stream>
void print_api_call(std::string_view url, const timeline_params& params, Stream& os)
{
	bool first = true;
	os << "GET " << url << '?';

	if (!params.max_id.empty())
	{
		os << "max_id=" << params.max_id;
		first = false;
	}

	if (!params.min_id.empty())
	{
		if (!first) { os << '&'; }
		os << "min_id=" << params.min_id;
		first = false;
	}

	if (!params.since_id.empty())
	{
		if (!first) { os << '&'; }
		os << "since_id=" << params.since_id;
	}

	os << '\n';
}
#endif
