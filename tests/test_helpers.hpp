#ifndef _TEST_HELPERS_HPP_
#define _TEST_HELPERS_HPP_

#include "../lib/options/filesystem.hpp"

#include <fstream>
#include <string>
#include <vector>

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
    test_file(fs::path name) : filename(name)
    {
        if (fs::exists(filename))
            fs::remove(filename);
        filename += ".bak";
        if (fs::exists(filename))
            fs::remove(filename);
    };

    ~test_file()
    {
        if (fs::exists(filename))
            fs::remove(filename);
        filename += ".bak";
        if (fs::exists(filename))
            fs::remove(filename);
    };

private:
    fs::path filename;
};

std::vector<std::string> inline read_lines(fs::path toread)
{
    std::ifstream fin(toread);
    std::vector<std::string> toreturn;

    for (std::string line; std::getline(fin, line);)
    {
        toreturn.push_back(line);
    }

    return toreturn;
}
#endif
