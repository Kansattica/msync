#include "queue_list.hpp"

bool Read(std::deque<std::string>& queued, std::string&& line)
{
	queued.push_back(std::move(line));
	return false;
}

void Write(std::deque<std::string>&& que, std::ofstream& of)
{
	for (const auto& str : que)
		if (!str.empty())
			of << str << '\n';
}
