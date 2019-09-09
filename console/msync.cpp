#include <iostream>

#include <print_logger.hpp>

#include "optionparsing/parseoptions.hpp"

int main(int argc, const char* argv[])
{
    std::cout << "Sup, nerds. Welcome to Mastosync.\n";
    print_logger<logtype::fileonly> pl;
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
