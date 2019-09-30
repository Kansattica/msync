#include "queue_list.hpp"

void Read(std::deque<std::string>& queued, std::string& line)
{
	queued.emplace_back(std::move(line));
}

void Write(std::string& str, std::ofstream& of)
{
	if (!str.empty())
		of << str << '\n';
}
