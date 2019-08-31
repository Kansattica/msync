#include <tclap/CmdLine.h>
#include <iostream>
#include <string>

#include "../lib/common/options.hpp"
#include "parseoptions.hpp"

void parse(int argc, char **argv)
{
    try
    {
        set_locations();
        TCLAP::CmdLine cmd("This is msync", ' ', "0.1");
        TCLAP::ValueArg<int> retryArg("r", "retries", "How many times to retry a failed operation", false, 3, "integer");
        TCLAP::MultiArg<std::string> accountsArg("r", "retries", "How many times to retry a failed operation", false, "account name");
        TCLAP::SwitchArg verboseArg("v", "verbose", "Produce more output", false);
        cmd.add(retryArg);
        cmd.add(verboseArg);
        cmd.parse(argc, argv);

        options.verbose = verboseArg.getValue();
        options.retries = retryArg.getValue();

    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}