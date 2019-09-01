#include <tclap/CmdLine.h>
#include <frozen/set.h>
#include <iostream>
#include <string>
#include <vector>

#include "../../lib/options/options.hpp"
#include "wideroutput.hpp"
#include "termwidth.hpp"
#include "parseoptions.hpp"

const std::string helpmessage = R"(msync is a command line utility for synchronizing with a Mastodon API-compatible server.
New account names must be fully specified, like: GoddessGrace@goodchristian.website.
Account names for accounts that you have logged into msync with can be shortened as long as it's unambiguous.
(for example, if you have GoddessGrace@goodchristian.website and GoodGraces@another.website, you could select them with -a God or -a Goo, respectively)
)";

const int columns = term_width();

// tclap isn't very const correct, huh
std::vector<std::string> validOptions {"accesstoken", "clientsecret", "authcode", "username", "password"};

void parse(int argc, char **argv)
{
    TCLAP::ValuesConstraint<std::string> allowedOptions(validOptions);
    try
    {
        TCLAP::CmdLine cmd(helpmessage, ' ', "0.1");
        TCLAP::ValueArg<int> retry("r", "retries", "How many times to retry a failed operation.", false, 3, "integer", cmd);
        TCLAP::ValueArg<std::string> option("", "option", "The name of the option to read or set for the given account.", false, "", &allowedOptions, cmd);
        TCLAP::ValueArg<std::string> setOption("", "set", "What to set the named option to.", false, "", "option value", cmd);
        TCLAP::MultiArg<std::string> accounts("a", "account", "The account or accounts to operate on.", false, "account name", cmd);
        TCLAP::SwitchArg verbose("v", "verbose", "Produce more output.", cmd, false);

        TCLAP::WiderOutput wideOut{columns};
        cmd.setOutput(&wideOut);

        cmd.parse(argc, argv);

        options.verbose = verbose.getValue();
        options.retries = retry.getValue();

        if (options.verbose)
            std::cout << "I detected your terminal width to be " << columns << " columns wide.\n";

        if (setOption.isSet() && !option.isSet())
            std::cerr << "If you're going to use --set, you have to provide an option to set.";

    }
    catch (const TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
}


