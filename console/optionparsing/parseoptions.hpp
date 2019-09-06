#include <string>
#include "../../lib/options/options.hpp"

enum class mode
{
    newuser,
    showopt,
    config,
    configsync,
    configlist,
    sync,
    gen,
    queue,
    help
};

struct parse_result
{
    bool okay;
    mode selected = mode::help;
    user_option toset;
    sync_settings syncset;
    list_operations listops;
    std::string optionval;
    std::string account;
};

parse_result parse(int argc, char **argv, bool silent = true);