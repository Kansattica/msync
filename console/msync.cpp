#include "../lib/common/printlog.hpp"

#include "../lib/options/options.hpp"

#include "optionparsing/parseoptions.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";
	//getgoodstuff(read_url("https://cybre.space/api/v1/instance"));

	parse(argc, argv);

	PrintLogger<logtype::verbose> pl;
	pl << "My executable is in " << options.executable_location << '\n';
	pl << "My current working directory is " << options.current_working_directory << '\n';
}
