#ifndef _MSYNC_UTIL_HPP_
#define _MSYNC_UTIL_HPP_
#include <string>
#include <string_view>
#include <optional>
#include <vector>

std::string make_api_url(const std::string_view instance_url, const std::string_view api_route);

struct parsed_account
{
    std::string username;
    std::string instance;
};

std::optional<parsed_account> parse_account_name(const std::string& name);

std::vector<std::string_view> split_string(const std::string_view str, char on);

template <typename Iterator, typename Stream>
void join_iterable(Iterator begin, Iterator end, const std::string_view sep, Stream& stream)
{
	for (auto it = begin; it != end;)
	{
		stream << *it;
		if (++it != end)
			stream << sep;
	}
}
#endif
