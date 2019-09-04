#include <clipp.h>

#include <iostream>
#include <string>

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

enum class mode
{
    config,
    sync,
    gen,
    queue,
    help
};

void parse(int argc, char **argv)
{
    using namespace std::string_literals;

    mode selected = mode::help;

    user_option toset;
    string optionval;
    string account;

    auto settableoptions = (one_of(
        command("accesstoken").set(toset, user_option::accesstoken),
        command("username").set(toset, user_option::username),
        command("password").set(toset, user_option::password),
        command("clientsecret").set(toset, user_option::clientsecret)));

    auto configMode = (command("config").set(selected, mode::config).doc("Set and show account-specific options."),
                       one_of(
                           (command("new").set(toset, user_option::newaccount)).doc("Register a new account with msync. Start here."),
                           command("showall").set(toset, user_option::show).doc("Print options for the specified account. If no account is specified, print options for all accounts."),
                           in_sequence(command("sync"),
                                       one_of(command("home").set(toset, user_option::home),
                                              command("dms").set(toset, user_option::dms),
                                              command("notifications").set(toset, user_option::notifications)),
                                       one_of(command("on").set(optionval, "T"s), command("off").set(optionval, "F"s)))
                               .doc("Whether to synchronize an account's home timeline, direct messages, and notifications."),
                           in_sequence(command("list"), one_of(command("add").set(toset, user_option::addlist), command("remove").set(toset, user_option::removelist)), value("list name", optionval)).doc("Add and remove lists from being synchronized for an account"),
                           (settableoptions & opt_value("value", optionval) % "If value given, set the specified option to that. Otherwise, show the corresponding value.")) %
                           "config commands");

    auto syncMode = ((command("sync").set(selected, mode::sync))
                         .doc("Synchronize your account[s] with their server[s]. Synchronizes all accounts unless one is specified with -a."),
                     (option("-r", "--retries") & value("times", options.retries)) % "Retry failed requests n times. (default: 3)");

    auto genMode = ((command("gen").set(selected, mode::gen) | command("generate").set(selected, mode::gen)).doc("Generate a post template in the current folder."));

    auto queueMode = ((command("queue").set(selected, mode::gen) | command("q").set(selected, mode::gen)).doc("Manage the queue of things to send."));

    auto universalOptions = (option("-a", "--account") & value("account", account).doc("The account name to operate on."),
                             option("-v", "--verbose").set(options.verbose).doc("Verbose mode. Program will be more chatty."));

    auto cli = (configMode | syncMode | genMode | queueMode | (command("help").set(selected, mode::help)), universalOptions);

    auto result = parse(argc, argv, cli);

    if (!result)
    {
        cout << make_man_page(cli, "msync").append_section("NOTES", helpmessage);
    }

    cout << static_cast<int>(selected) << '\n';
    cout << static_cast<int>(toset) << '\n';
}
