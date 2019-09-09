#ifndef _OPTIONS_H_
#define _OPTIONS_H_
#include "global_options.hpp"

#include <variant>
#include <string>

extern global_options options;

std::variant<const user_options* const, const std::string> select_account(const std::string& name);
#endif