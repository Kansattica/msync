#ifndef _GLOBAL_OPTIONS_HPP_
#define _GLOBAL_OPTIONS_HPP_

#include "user_options.hpp"
#include <filesystem.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

struct global_options
{
public:
    const fs::path executable_location = get_exe_location();

    std::unordered_map<std::string, user_options> accounts = read_accounts();

	std::pair<const std::string, user_options>& add_new_account(std::string name);
	std::pair<const std::string, user_options>* select_account(const std::string_view name);

private:
    fs::path get_exe_location();
    std::unordered_map<std::string, user_options> read_accounts();
};

global_options& options();
#endif