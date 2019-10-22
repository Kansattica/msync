#include "queue_list.hpp"

void Read(std::deque<std::string>& queued, std::string&& line)
{
	queued.push_back(std::move(line));
}

void Write(std::deque<std::string>&& que, std::ofstream& of)
{
	for (const auto& str : que)
		if (!str.empty())
			of << str << '\n';
}
