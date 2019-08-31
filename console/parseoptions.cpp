#include <tclap/CmdLine.h>
#include <iostream>
#include <string>

#include "../lib/common/options.hpp"
#include "wideroutput.hpp"
#include "termwidth.hpp"
#include "parseoptions.hpp"

const std::string helpmessage = R"(msync is a command line utility for synchronizing with a Mastodon API-compatible server.
New account names must be fully specified, like: GoddessGrace@goodchristian.website.
Account names for accounts that you have logged into msync with can be shortened as long as it's unambiguous.
(for example, if you have GoddessGrace@goodchristian.website and GoodGraces@another.website, you could select them with -a God or -a Goo, respectively)
)";

const int columns = term_width();
TCLAP::WiderOutput fixedOutput {columns};

void parse(int argc, char **argv)
{
    try
    {
        set_locations();
        TCLAP::CmdLine cmd(helpmessage, ' ', "0.1");
        TCLAP::ValueArg<int> retryArg("r", "retries", "How many times to retry a failed operation", false, 3, "integer");
        TCLAP::MultiArg<std::string> accountsArg("a", "account", "The account or accounts to operate on.", false, "account name");
        TCLAP::SwitchArg verboseArg("v", "verbose", "Produce more output", false);
        cmd.add(retryArg);
        cmd.add(verboseArg);
        cmd.add(accountsArg);

        cmd.setOutput(&fixedOutput);
        cmd.parse(argc, argv);

        options.verbose = verboseArg.getValue();
        options.retries = retryArg.getValue();

        if (options.verbose)
        {
            std::cout << "I detected your terminal width to be " << columns << " columns wide.\n";
        }
    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}

