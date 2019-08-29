#include <tclap/CmdLine.h>
#include <iostream>
#include <optional>

#include "parseoptions.hpp"

std::optional<int> parse(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine cmd("This is msync", ' ', "0.1");
        TCLAP::ValueArg<int> numArg("n", "num", "Number to check", false, 0, "integer");
        cmd.add(numArg);
        cmd.parse(argc, argv);

        if (numArg.isSet())
            return numArg.getValue();
    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return {};
}