#ifndef _OPTION_FILE_HPP_
#define _OPTION_FILE_HPP_

#include <map> //use an ordered map so keys don't get shuffled around between runs
#include <string>

#include "../filebacked/file_backed.hpp"

using std::string;

void Read(std::map<std::string, std::string>&, std::string&&);
void Write(std::pair<const std::string, std::string>&, std::ofstream&);

using option_file = file_backed<std::map<std::string, std::string>, std::pair<const std::string, std::string>, Read, Write>;

#endif