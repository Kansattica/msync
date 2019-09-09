#include <clipp.h>

#include <iostream>

#include "../../lib/options/options.hpp"
#include "parseoptions.hpp"

using namespace clipp;
using std::cout;
using std::string;

const std::string helpmessage = R"(msync is a command line utility for synchronizing with a Mastodon API-compatible server.
Account names for accounts that you have logged into msync with can be shortened as long as it's unambiguous.
(for example, if you have GoddessGrace@goodchristian.website and GoodGraces@another.website, you could select them with -a God or -a Goo, respectively)
The account name can be left out of most commands entirely if you only have one account registered with msync.
If this is your first time, try running:

msync config new -a [account name]

New account names must be fully specified, like: GoddessGrace@goodchristian.website
)";

parse_result parse(const int argc, const char *argv[], const bool silent)
{
    using namespace std::string_literals;

    parse_result ret;

    auto settableoptions = (one_of(
        command("accesstoken").set(ret.toset, user_option::accesstoken).set(ret.selected, mode::showopt),
        command("username").set(ret.toset, user_option::username).set(ret.selected, mode::showopt),
        command("password").set(ret.toset, user_option::password).set(ret.selected, mode::showopt),
        command("clientsecret").set(ret.toset, user_option::clientsecret).set(ret.selected, mode::showopt)));

    auto newaccount = (command("new").set(ret.selected, mode::newuser)).doc("Register a new account with msync. Start here.");
    auto configMode = (command("config").set(ret.selected, mode::config).doc("Set and show account-specific options.") &
                       one_of(
                           command("showall").set(ret.selected, mode::showopt).doc("Print options for the specified account. If no account is specified, print options for all accounts."),
                           in_sequence(command("sync").set(ret.selected, mode::configsync),
                                       one_of(command("home").set(ret.toset, user_option::home),
                                              command("dms").set(ret.toset, user_option::dms),
                                              command("notifications").set(ret.toset, user_option::notifications)),
                                       one_of(command("newest").set(ret.syncset, sync_settings::newest_first),
                                              command("oldest").set(ret.syncset, sync_settings::oldest_first),
                                              command("off").set(ret.syncset, sync_settings::off)))
                               .doc("Whether to synchronize an account's home timeline, direct messages, and notifications, and whether to do it newest first, oldest first, or not at all."),
                           in_sequence(command("list").set(ret.selected, mode::configlist),
                                       one_of(command("add").set(ret.listops, list_operations::add),
                                              command("remove").set(ret.listops, list_operations::remove)),
                                       value("list name", ret.optionval))
                               .doc("Add and remove lists from being synchronized for an account"),
                           (settableoptions & opt_value("value", ret.optionval).set(ret.selected, mode::config) % "If given, set the specified option to that. Otherwise, show the corresponding value.")) %
                           "config commands");

    auto syncMode = ((command("sync").set(ret.selected, mode::sync)
                         .doc("Synchronize your account[s] with their server[s]. Synchronizes all accounts unless one is specified with -a.")) &
                     (option("-r", "--retries") & value("retries", ret.syncopts.retries)) % "Retry failed requests n times. (default: 3)");

    auto genMode = ((command("gen").set(ret.selected, mode::gen) | command("generate").set(ret.selected, mode::gen)).doc("Generate a post template in the current folder."));

    auto queueMode = ((command("queue").set(ret.selected, mode::queue) | command("q").set(ret.selected, mode::queue)).doc("Manage the queue of things to send."));

    auto universalOptions = ((option("-a", "--account") & value("account", ret.account)).doc("The account name to operate on."),
                             option("-v", "--verbose").set(options.verbose).doc("Verbose mode. Program will be more chatty."));

    auto cli = (newaccount | configMode | syncMode | genMode | queueMode | (command("help").set(ret.selected, mode::help)), universalOptions);

    //skip the first result.
    //we do it this way because C++11 and later don't like it when you turn a string literal into a char*, so we have to use the iterator interface
    auto result = clipp::parse(argv+1, argc+argv, cli);

    if (!silent && (!result || ret.selected == mode::help))
    {
        cout << make_man_page(cli, "msync").append_section("NOTES", helpmessage);
    }

    ret.okay = static_cast<bool>(result);

    return ret;
}
