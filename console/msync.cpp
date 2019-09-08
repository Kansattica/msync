#include <iostream>

#include <printlogger.hpp>

#include "optionparsing/parseoptions.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";
	PrintLogger<logtype::fileonly> pl;
	pl << "--- Mastosync started ---\n";

	auto parsed = parse(argc, argv, false);
	
	switch (parsed.selected)
	{
		case mode::newuser:

		default:
			std::cout << "[option not implemented]\n";
	}

	pl << "--- Mastosync finished normally ---\n";
}
