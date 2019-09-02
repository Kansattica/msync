#include <tclap/CmdLine.h>
#include <frozen/set.h>
#include <iostream>
#include <string>

#include "../lib/options/options.hpp"
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
        TCLAP::CmdLine cmd(helpmessage, ' ', "0.1");
        TCLAP::ValueArg<int> retry("r", "retries", "How many times to retry a failed operation.", false, 3, "integer", cmd);
        TCLAP::ValueArg<std::string> set("", "set", "The name of the option to set for the given account. Acceptable values are 'accesstoken', 'clientsecret', 'authcode', 'username', 'password'", false, "", "option name", cmd);
        TCLAP::MultiArg<std::string> accounts("a", "account", "The account or accounts to operate on.", false, "account name", cmd);
        TCLAP::SwitchArg verbose("v", "verbose", "Produce more output.", cmd, false);

        cmd.setOutput(&fixedOutput);
        cmd.parse(argc, argv);

        options.verbose = verbose.getValue();
        options.retries = retry.getValue();

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

