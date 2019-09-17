#include "option_file.hpp"

#include <print_logger.hpp>

using std::getline;
using std::string;

option_file::option_file(fs::path filename) : optionfilename(filename)
{
    print_logger<logtype::verbose> logger;
    std::ifstream optionfile(optionfilename);
    for (string line; getline(optionfile, line);)
    {
        logger << "Parsing line: " << line << '\n';

        const auto first_non_whitespace = line.find_first_not_of(" \t\r\n");

        if (first_non_whitespace == string::npos)
            continue; // blank line?

        if (line[first_non_whitespace] == '#')
            continue; //skip comments

        const auto equals = line.find_first_of('=');
        const auto [it, success] = parsed_options.insert({line.substr(0, equals), line.substr(equals + 1)});
        if (!success)
            logger << "Duplicate key " << it->first << " found. Ignoring.\n";
    }
}

option_file::~option_file()
{
    if (optionfilename == "")
        return; // optionfile got moved from, so the new version will save it

    print_logger<logtype::verbose> logger;

    fs::path backup(optionfilename);
    backup += ".bak";

    if (fs::exists(optionfilename))
    {
        fs::rename(optionfilename, backup);
        logger << "Saved backup to " << backup << '\n';
    }

    ofstream of(optionfilename);
    for (auto& kvp : parsed_options)
    {
        if (!kvp.second.empty()) //don't serialize
            of << kvp.first << '=' << kvp.second << '\n';
    }

    logger << "Saved " << optionfilename << '\n';
}
