#ifndef _QUEUE_LIST_HPP_
#define _QUEUE_LIST_HPP_

#include <deque>
#include <string>

#include "../filebacked/file_backed.hpp"

bool Read(std::deque<std::string>&, std::string&&);
void Write(std::deque<std::string>&&, std::ofstream&);

using queue_list = file_backed<std::deque<std::string>, Read, Write>;
using readonly_queue_list = file_backed<std::deque<std::string>, Read, Write, true, true, true>;
#endif
