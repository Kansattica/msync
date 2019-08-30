#include <tclap/CmdLine.h>
#include <iostream>
#include <vector>

#include "parseoptions.hpp"

std::vector<int> parse(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine cmd("This is msync", ' ', "0.1");
        TCLAP::MultiArg<int> numArg("n", "num", "Number to check", false, "integer");
        cmd.add(numArg);
        cmd.parse(argc, argv);

        return numArg.getValue();
    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return std::vector<int>();
}