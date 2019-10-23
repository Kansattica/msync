#include "parseoptions.hpp"
#include <clipp.h>                               // for parameter, command
#include <iostream>                              // for ostream, cout
#include <print_logger.hpp>                      // for verbose_logs
#include "../../lib/postfile/outgoing_post.hpp"  // for post_content
#include "../../lib/queue/queues.hpp"            // for queues, queues::boost

using namespace clipp;

constexpr auto helpmessage = R"(msync is a command line utility for synchronizing with a Mastodon API-compatible server.
Account names for accounts that you have logged into msync with can be shortened as long as it's unambiguous.
(for example, if you have GoddessGrace@goodchristian.website and GoodGraces@another.website, you could select them with -a God or -a Goo, respectively)
The account name can be left out of most commands entirely if you only have one account registered with msync.
If this is your first time, try running:

msync new -a [account name]

New account names must be fully specified, like: GoddessGrace@goodchristian.website
)";

parse_result parse(const int argc, const char* argv[], const bool silent)
{
    using namespace std::string_literals;

    parse_result ret;

    const auto settableoptions = (one_of(
        command("accesstoken").set(ret.toset, user_option::access_token).set(ret.selected, mode::showopt),
        command("authcode").set(ret.toset, user_option::auth_code).set(ret.selected, mode::showopt),
        command("accountname").set(ret.toset, user_option::account_name).set(ret.selected, mode::showopt),
        command("instanceurl").set(ret.toset, user_option::instance_url).set(ret.selected, mode::showopt),
        command("clientid").set(ret.toset, user_option::client_id).set(ret.selected, mode::showopt),
        command("clientsecret").set(ret.toset, user_option::client_secret).set(ret.selected, mode::showopt)));

    const auto newaccount = (command("new").set(ret.selected, mode::newuser)).doc("Register a new account with msync. Start here.");
    const auto configMode = (command("config").set(ret.selected, mode::config).doc("Set and show account-specific options.") &
                       one_of(
                           command("showall").set(ret.selected, mode::showallopt).doc("Print all options for the specified account."),
                           in_sequence(command("sync").set(ret.selected, mode::configsync),
                                       one_of(command("home").set(ret.toset, user_option::pull_home),
                                              command("dms").set(ret.toset, user_option::pull_dms),
                                              command("notifications").set(ret.toset, user_option::pull_notifications)),
                                       one_of(command("newest").set(ret.sync_opts.mode, sync_settings::newest_first),
                                              command("oldest").set(ret.sync_opts.mode, sync_settings::oldest_first),
                                              command("off").set(ret.sync_opts.mode, sync_settings::dont_sync)))
                               .doc("Whether to synchronize an account's home timeline, direct messages, and notifications, and whether to do it newest first, oldest first, or not at all."),
                           in_sequence(command("list").set(ret.selected, mode::configlist),
                                       one_of(command("add").set(ret.listops, list_operations::add),
                                              command("remove").set(ret.listops, list_operations::remove)),
                                       value("list name", ret.optionval))
                               .doc("Add and remove lists from being synchronized for an account"),
                           (settableoptions & opt_value("value", ret.optionval).set(ret.selected, mode::config) % "If given, set the specified option to that. Otherwise, show the corresponding value.")) %
                           "config commands");

    const auto syncMode = ((command("sync").set(ret.selected, mode::sync).doc("Synchronize your account[s] with their server[s]. Synchronizes all accounts unless one is specified with -a.")) &
                     (option("-r", "--retries") & value("retries", ret.sync_opts.retries)) % "Retry failed requests n times. (default: 3)",
		             one_of(
						 option("-s", "--send-only").set(ret.sync_opts.get, false).doc("Only send queued messages, don't download anything."),
						 option("-g", "--get-only").set(ret.sync_opts.send, false).doc("Only download posts, don't send anything from queues.")
					 ));

	const auto genMode = (command("gen", "generate").set(ret.selected, mode::gen).doc("Generate a post template in the current folder. Edit this file afterwards to add a body.") &
					 (
						(option("-f", "--file", "--attach") & values(match::prefix_not("-"), "file path", ret.gen_opt.post.attachments)).doc("Attach these files to the post."),
						(option("-d", "--description") & values("file description", ret.gen_opt.post.descriptions)).doc("Associate this description with the corresponding file."),
						(option("-o", "--output") & value("filename", ret.gen_opt.filename)).doc("Specify an output file. Default is post.msync."),
						(option("-r", "--reply-to") & value("reply_to", ret.gen_opt.post.reply_to_id)).doc("Reply to the specified post ID."),
						(option("-c", "--content-warning") & value("warning", ret.gen_opt.post.content_warning)).doc("Set a content warning (or subject) for the post.")
					) % "generate options");

	const auto queueMode = (command("q", "queue").set(ret.selected, mode::queue).doc("Manage queued favs, boosts, and posts") &
                          one_of(option("-r", "--remove").set(ret.queue_opt.to_do, queue_action::remove).doc("Remove the post ids or filenames from the queue instead of adding them. If not in the queue, queue unfaving, unboosting, or deleting the post so it happens on next sync."),
                                 option("-c", "--clear").set(ret.queue_opt.to_do, queue_action::clear).doc("Remove everything in the specified queue."),
								 option("-p", "--print").set(ret.queue_opt.to_do, queue_action::print).doc("Print everything in the specified queue. The first item is on top.")) %
                              "queue options",
                      one_of(
                          command("fav").set(ret.queue_opt.selected, queues::fav) & opt_values("post ids", ret.queue_opt.queued),
                          command("boost").set(ret.queue_opt.selected, queues::boost) & opt_values("post ids", ret.queue_opt.queued),
                          command("post").set(ret.queue_opt.selected, queues::post) & opt_values("filenames", ret.queue_opt.queued))
                          .doc("queue commands"));

    const auto universalOptions = ((option("-a", "--account") & value("account", ret.account)).doc("The account name to operate on."),
                             option("-v", "--verbose").set(verbose_logs).doc("Verbose mode. Program will be more chatty."));

	const auto cli = (newaccount | configMode | syncMode | genMode | queueMode | (command("help").set(ret.selected, mode::help)), universalOptions);

    //skip the first result.
    //we do it this way because C++11 and later don't like it when you turn a string literal into a char*, so we have to use the iterator interface
    auto result = clipp::parse(argv + 1, argc + argv, cli);

    if (!result || ret.selected == mode::help)
    {
        if (!silent)
            std::cout << make_man_page(cli, "msync").append_section("NOTES", helpmessage);

        ret.selected = mode::help; //possible for, say, config to be set but still be a parse fail

        //clipp::debug::print(cout, result);
    }

    ret.okay = static_cast<bool>(result);

    return ret;
}
