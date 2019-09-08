#include <string>
#include "../../lib/options/options.hpp"
#include "selection_types.hpp"

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
    union { //only one or the other will be set
        sync_settings syncset;
        list_operations listops;
        sync_options syncopts;
    }; 
    std::string optionval;
    std::string account;
};

parse_result parse(int argc, const char *argv[], bool silent = true);