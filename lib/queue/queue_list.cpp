#include "queue_list.hpp"

#include <array>
#include <string_view>

// not +1 because unknown doesn't get a string
constexpr std::array<std::string_view, static_cast<uint8_t>(api_route::unknown)> ROUTE_NAMES = {
	"FAV", "UNFAV", "BOOST", "UNBOOST", "POST", "UNPOST"
};

std::string_view print_route(api_route route)
{
	return ROUTE_NAMES[static_cast<uint8_t>(route)];
}

api_route parse_route(std::string_view str)
{
	for (auto route = api_route(0); route < api_route::unknown; route = api_route(static_cast<uint8_t>(route) + 1))
	{
		if (str == print_route(route))
			return route;
	}

	return api_route::unknown;
}

bool Read(std::deque<api_call>& queued, std::string&& line)
{
	const auto first_space = line.find(' ');

	const auto parsed_route = parse_route(std::string_view(line.data(),
			first_space == std::string::npos ? line.size() : first_space));

	if (first_space != std::string::npos)
		line.erase(0, first_space + 1);
	else
		line.clear();

	queued.push_back(api_call{ parsed_route, std::move(line) });
	return false;
}

void Write(std::deque<api_call>&& que, std::ofstream& of)
{
	for (const auto& call : que)
	{
		if (call.queued_call == api_route::unknown) continue;

		of << print_route(call.queued_call);

		if (!call.argument.empty())
			of << ' ' << call.argument;

		of << '\n';
	}
}

bool operator== (const api_call& rhs, const api_call& lhs)
{
	return rhs.queued_call == lhs.queued_call && rhs.argument == lhs.argument;
}

