#include "queue_list.hpp"

#include <fstream>

void Read(std::string&& line, std::deque<std::string>& queued)
{
    queued.emplace_back(std::move(line));
}

void Write(std::deque<std::string>&& queued, std::ofstream& of)
{
    while (!queued.empty())
    {
        of << queued.front() << '\n';
        queued.pop_front();
    }
}
