#include <iostream>
#include <array>
#include <vector>
#include "../lib/common/net.hpp"
#include "../lib/parse/inoutbox.hpp"

#include "../lib/common/options.hpp"

#include "parseoptions.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";
	//getgoodstuff(read_url("https://cybre.space/api/v1/instance"));

	if (options.verbose)
	{
		std::cout << "I think that my executable is in " << options.executable_location << "\nand my current working directory is " << options.current_working_directory << std::endl;
	}
}
