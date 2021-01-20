#ifndef QUEUE_LIST_HPP
#define QUEUE_LIST_HPP

#include <deque>
#include <string>

#include "../filebacked/file_backed.hpp"

enum class api_route : uint8_t
{
	fav,
	unfav,
	boost,
	unboost,
	bookmark,
	unbookmark,
	post,
	unpost, // you might call it "delete post"
	context,
	unknown,
};

struct api_call
{
	api_route queued_call;
	std::string argument;
};

bool operator== (const api_call& rhs, const api_call& lhs);

std::string_view print_route(api_route route);

bool Read(std::deque<api_call>&, std::string&&);
void Write(std::deque<api_call>&&, std::ofstream&);

using queue_list = file_backed<std::deque<api_call>, Read, Write>;
using readonly_queue_list = file_backed<std::deque<api_call>, Read, Write, true, true, true>;
#endif
