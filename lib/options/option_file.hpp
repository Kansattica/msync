#ifndef _OPTION_FILE_HPP_
#define _OPTION_FILE_HPP_

#include <map> //use an ordered map so keys don't get shuffled around between runs
#include <string>

#include "../backed_file/backed_file.hpp"

void Read(std::string&& line, std::map<std::string, std::string>& parsed_options);
void Write(std::map<std::string, std::string>&&, std::ofstream&);

typedef backed_file<std::map<std::string, std::string>, Read, Write> option_file;

#endif