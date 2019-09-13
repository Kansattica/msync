#include "../../lib/options/user_options.hpp"
#include "selection_types.hpp"
#include <string>
#include <vector>

enum class mode
{
    newuser,
    showopt,
    showallopt,
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
    // maybe later, do a union or variant for the mutually exclusive ones
    sync_settings syncset;
    list_operations listops;
    sync_options syncopts;
    to_queue queueaction;
    std::vector<std::string> queued; 
    bool removeFromQueue = false;
    bool clearQueue = false;
    std::string optionval;
    std::string account;
};

parse_result parse(int argc, const char* argv[], bool silent = true);