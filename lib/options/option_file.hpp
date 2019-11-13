#ifndef _OPTION_FILE_HPP_
#define _OPTION_FILE_HPP_

#include <map> //use an ordered map so keys don't get shuffled around between runs
#include <string>

#include "../filebacked/file_backed.hpp"

bool Read(std::map<std::string, std::string, std::less<>>&, std::string&&);
void Write(std::map<std::string, std::string, std::less<>>&&, std::ofstream&);

using option_file = file_backed<std::map<std::string, std::string, std::less<>>, Read, Write>;

#endif