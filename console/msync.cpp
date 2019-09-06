#include <printlogger.hpp>

#include "optionparsing/parseoptions.hpp"

int main(int argc, const char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";

	auto parsed = parse(argc, argv, false);
}
