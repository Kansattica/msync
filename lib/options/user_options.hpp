#ifndef _USER_OPTIONS_HPP_
#define _USER_OPTIONS_HPP_

#include <string>

#include "filesystem.hpp"
#include "option_enums.hpp"
#include "option_file.hpp"

struct user_options
{
public:
    user_options(fs::path toread) : backing(toread){};

    const std::string* get_option(user_option toget) const;
    sync_settings get_sync_option(user_option toget) const;

    void set_option(user_option toset, std::string value);
    void set_option(user_option toset, list_operations value);
    void set_option(user_option toset, sync_settings value);

private:
    option_file backing;
};
#endif
