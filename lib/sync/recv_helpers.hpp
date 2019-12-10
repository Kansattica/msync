#ifndef _RECV_HELPERS_HPP_
#define _RECV_HELPERS_HPP_

#include <string_view>
#include <string>

#include "../constants/constants.hpp"

#include "read_response.hpp"

enum class to_get { notifications, home, dms, lists };

struct recv_parameters { user_option last_id_setting; user_option sync_setting; std::string_view route; std::string_view filename; };

constexpr std::string_view home_route{ "/api/v1/timelines/home" };
constexpr std::string_view notifications_route{ "/api/v1/notifications" };

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

template <typename Number>
Number clamp_or_default(Number input, Number maxdefault)
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

	os << '\n';
}
#endif
