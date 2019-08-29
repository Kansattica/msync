#include <tclap/CmdLine.h>
#include <iostream>
#include <tuple>

#include "parseoptions.hpp"

std::tuple<int, bool> parse(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine cmd("This is msync", ' ', "0.1");
        TCLAP::ValueArg<int> numArg("n", "num", "Number to check", false, 0, "integer");
        cmd.add(numArg);
        cmd.parse(argc, argv);

        if (numArg.isSet())
            return std::make_tuple(numArg.getValue(), true);
    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return std::make_tuple(0, false);
}