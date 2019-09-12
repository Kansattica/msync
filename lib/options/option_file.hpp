#ifndef _OPTION_FILE_HPP_
#define _OPTION_FILE_HPP_

#include <filesystem.hpp>
#include <map> //use an ordered map so keys don't get shuffled around between runs
#include <string>

using std::string;

struct option_file
{
public:
    std::map<string, string> parsed_options;
    option_file(fs::path filename);
    ~option_file();

    // can be moved
    option_file(option_file&& other) noexcept // move constructor
        : optionfilename(std::move(other.optionfilename)), parsed_options(std::move(other.parsed_options))
    {
    }

    option_file& operator=(option_file&& other) noexcept // move assignment
    {
        std::swap(parsed_options, other.parsed_options);
        std::swap(optionfilename, other.optionfilename);
        return *this;
    }

    // optionfiles can't be copied
    option_file(const option_file& other) = delete;            // copy constructor
    option_file& operator=(const option_file& other) = delete; // copy assignment

private:
    fs::path optionfilename;
};

#endif