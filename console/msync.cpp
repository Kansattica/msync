#include <printlogger.hpp>

#include "../lib/options/options.hpp"
#include "../lib/options/option_file.hpp"

#include "optionparsing/parseoptions.hpp"


void makeoption()
{
	option_file coolopts("regularfile.options");

	coolopts.parsed_options.insert({"goodbye", "stranger"});
}

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";

	parse(argc, argv);

	PrintLogger<logtype::verbose> pl;
	pl << "Looking for options in " << options.executable_location << '\n';
	pl << "My current working directory for logs and arguments is " << options.current_working_directory << '\n';

	makeoption();
}
