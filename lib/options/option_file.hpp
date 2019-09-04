#ifndef _OPTION_FILE_HPP_
#define _OPTION_FILE_HPP_

#include <string>
#include <map> //use an ordered map so keys don't get shuffled around between runs
#include "filesystem.hpp"

using std::string;

struct option_file
{
public:
    std::map<string, string> parsed_options;
    option_file(fs::path filename);
    ~option_file();

private:
    fs::path optionfilename;
};

#endif