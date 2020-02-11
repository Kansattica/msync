#ifndef _MSYNC_UTIL_HPP_
#define _MSYNC_UTIL_HPP_
#include <string>
#include <string_view>
#include <utility>
#include <optional>
#include <vector>

std::string make_api_url(std::string_view instance_url, std::string_view api_route);

struct parsed_account
{
    std::string username;
    std::string instance;
};

std::optional<parsed_account> parse_account_name(const std::string& name);

std::string clean_up_html(std::string_view to_strip);
std::string& bulk_replace_mentions(std::string& str, const std::vector<std::pair<std::string_view, std::string_view>>& to_replace);

template <bool allowEmpty = false>
std::vector<std::string_view> split_string(const std::string_view tosplit, const char on)
{
	size_t start = 0;
	std::vector<std::string_view> toreturn;
	for (size_t i = 0; i < tosplit.size(); i++)
	{
		if (tosplit[i] == on)
		{
			if constexpr (allowEmpty)
			{
				toreturn.push_back(tosplit.substr(start, (i - start)));
			}
			else
			{
				if (i > start)
					toreturn.push_back(tosplit.substr(start, (i - start)));
			}
			start = i + 1;
		}
	}

	// if we have stuff left over at the end
	auto end = tosplit.substr(start);

	if (!end.empty())
	{
		toreturn.push_back(std::move(end));
	}

	return toreturn;
}

template <typename Iterator, typename Stream, typename Sep>
void join_iterable(Iterator begin, Iterator end, const Sep& sep, Stream& stream)
{
	for (auto it = begin; it != end;)
	{
		stream << *it;
		if (++it != end)
			stream << sep;
	}
}

#endif
