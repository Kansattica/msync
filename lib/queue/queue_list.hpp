#ifndef _QUEUE_LIST_HPP_
#define _QUEUE_LIST_HPP_

#include <deque>
#include <string>

#include "../backed_file/backed_file.hpp"

void Read(std::string&& line, std::deque<std::string>& parsed_options);
void Write(std::deque<std::string>&&, std::ofstream&);

typedef backed_file<std::deque<std::string>, Read, Write> queue_list;

#endif
