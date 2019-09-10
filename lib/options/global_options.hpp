#ifndef _GLOBAL_OPTIONS_HPP_
#define _GLOBAL_OPTIONS_HPP_

#include "filesystem.hpp"
#include "user_options.hpp"

#include <string>
#include <unordered_map>

struct global_options
{
public:
    bool verbose = false;
    const fs::path executable_location = get_exe_location();
    const fs::path current_working_directory = fs::current_path();

    std::unordered_map<std::string, user_options> accounts = read_accounts();

    bool add_new_account(std::string name);

private:
    fs::path get_exe_location();
    std::unordered_map<std::string, user_options> read_accounts();
};

extern global_options options;
#endif