#include <iostream>

#include <msync_exception.hpp>
#include <print_logger.hpp>

#include "optionparsing/parseoptions.hpp"

int main(int argc, const char* argv[])
{
    std::cout << "Sup, nerds. Welcome to Mastosync.\n";
    print_logger<logtype::fileonly> pl;
    print_logger<logtype::normal> plerr;
    pl << "--- msync started ---\n";

    auto parsed = parse(argc, argv, false);

    try
    {
        switch (parsed.selected)
        {
        case mode::newuser:

        default:
            std::cout << "[option not implemented]\n";
        }
    }
    catch (const msync_exception& e)
    {
        plerr << "An error occurred: " << e.what() << '\n';
    }

    pl << "--- msync finished normally ---\n";
}
