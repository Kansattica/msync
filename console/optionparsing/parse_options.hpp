#include "selection_types.hpp"
#include <string>

enum class mode
{
	newuser,
	showopt,
	setdefault,
	showallopt,
	config,
	configsync,
	configlist,
	sync,
	gen,
	queue,
	help,
	version,
	yeehaw,
	location,
	license
};

struct parse_result
{
	bool okay = false;
	mode selected = mode::help;
	user_option toset;
	// maybe later, do a union or variant for the mutually exclusive ones
	list_operations listops;
	sync_options sync_opts;
	queue_options queue_opt;
	gen_options gen_opt;
	std::string optionval;
	std::string account;
};

const parse_result& parse(int argc, const char* argv[], bool silent = true);
