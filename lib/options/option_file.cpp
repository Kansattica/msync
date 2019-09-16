#include "option_file.hpp"

#include <print_logger.hpp>

using std::getline;
using std::string;

void Read(std::string&& line, std::map<string, string>& parsed_options)
{
    const auto equals = line.find_first_of('=');
    const auto [it, success] = parsed_options.insert({line.substr(0, equals), line.substr(equals + 1)});
}

void Write(std::map<std::string, std::string>&& parsed_options, std::ofstream& of)
{
    for (auto& kvp : parsed_options)
    {
        if (!kvp.second.empty()) //don't serialize
            of << kvp.first << '=' << kvp.second << '\n';
    }
}
