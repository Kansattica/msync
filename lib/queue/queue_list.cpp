#include "queue_list.hpp"

#include <fstream>

// this is almost the same implementation as option_file
// it'd be nice if I could only have one piece of code for both
// I tried having a base class that these guys inherit from, but you can't call virtual functions in the constructor
// Try some template magic later?
queue_list::queue_list(fs::path filename) : listfilename(filename)
{
    std::ifstream listfile(listfilename);
    for (string line; std::getline(listfile, line);)
    {
        const auto first_non_whitespace = line.find_first_not_of(" \t\r\n");

        if (first_non_whitespace == string::npos)
            continue; // blank line?

        if (line[first_non_whitespace] == '#')
            continue; //skip comments

        queued.emplace_back(std::move(line));
    }
}

queue_list::~queue_list()
{
    if (listfilename == "")
        return; // we got moved from, so the new version will save it

    fs::path backup = listfilename;
    backup += ".bak";

    if (fs::exists(listfilename))
    {
        fs::rename(listfilename, backup);
    }

    std::ofstream of(listfilename);
    while (!queued.empty())
    {
		if (!queued.front().empty())
			of << queued.front() << '\n';
        queued.pop_front();
    }
}
