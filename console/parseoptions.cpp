#include <tclap/CmdLine.h>
#include <iostream>
#include <vector>

#include "../lib/common/options.hpp"
#include "parseoptions.hpp"

std::vector<int> parse(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine cmd("This is msync", ' ', "0.1");
        TCLAP::MultiArg<int> numArg("n", "num", "Number to check", false, "integer");
        TCLAP::SwitchArg verboseArg("v", "verbose", "Produce more output", false);
        cmd.add(numArg);
        cmd.add(verboseArg);
        cmd.parse(argc, argv);

        options.verbose = verboseArg.getValue();
        return numArg.getValue();
    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return std::vector<int>();
}