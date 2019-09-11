#ifndef _USER_OPTIONS_HPP_
#define _USER_OPTIONS_HPP_

#include <string>

#include "filesystem.hpp"

#include "option_file.hpp"

#include "option_enums.hpp"

struct user_options
{
public:
    user_options(fs::path toread) : backing(toread){};

    const std::string* get_option(user_option toget) const;
    void set_option(user_option toset, std::string value);

private:
    option_file backing;
};
#endif
