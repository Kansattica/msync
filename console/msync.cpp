#include <iostream>

#include <exception>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <string>
#include <algorithm>

#include "../lib/options/global_options.hpp"
#include "../lib/options/option_enums.hpp"
#include "../lib/options/user_options.hpp"
#include "../lib/postfile/outgoing_post.hpp"
#include "../lib/queue/queues.hpp"
#include "../lib/sync/send.hpp"
#include "../lib/net/net.hpp"
#include "../lib/util/util.hpp"
#include "newaccount.hpp"
#include "optionparsing/parseoptions.hpp"

std::pair<const std::string, user_options>& assume_account(std::pair<const std::string, user_options>* user);

void print_stringptr(const std::string* toprint);

template <typename T>
void uniqueify(T& toprint);

template <typename T>
void print_iterable(const T& vec);

int main(int argc, const char* argv[])
{
	plfile() << "--- msync started ---\n";

	auto parsed = parse(argc, argv, false);

    auto user = options().select_account(parsed.account);
    try
    {
        switch (parsed.selected)
        {
        case mode::newuser:
            make_new_account(parsed.account);
            break;
        case mode::showopt:
            print_stringptr(assume_account(user).second.get_option(parsed.toset));
            break;
        case mode::showallopt:
			for (auto opt = user_option(0); opt <= user_option::pull_notifications; opt = user_option(static_cast<int>(opt) + 1))
			{
				pl() << USER_OPTION_NAMES[static_cast<int>(opt)] << ": ";
				if (opt < user_option::pull_home)
					print_stringptr(assume_account(user).second.get_option(opt));
				else
					pl() << SYNC_SETTING_NAMES[static_cast<int>(assume_account(user).second.get_sync_option(opt))];
				pl() << '\n';
			}
			pl() << "Accounts registered: ";
			{
				const auto accountnames = options().all_accounts();
				join_iterable(accountnames.begin(), accountnames.end(), ", ", pl());
			}
            break;
        case mode::config:
            assume_account(user).second.set_option(parsed.toset, parsed.optionval);
            break;
        case mode::configsync:
            assume_account(user).second.set_option(parsed.toset, parsed.sync_opts.mode);
            break;
        case mode::configlist:
            assume_account(user).second.set_option(parsed.toset, parsed.listops);
            break;
		case mode::queue:
			uniqueify(parsed.queue_opt.queued);
			switch (parsed.queue_opt.to_do)
			{
			case queue_action::add:
				enqueue(parsed.queue_opt.selected, assume_account(user).first, parsed.queue_opt.queued);
				break;
			case queue_action::remove:
				dequeue(parsed.queue_opt.selected, assume_account(user).first, std::move(parsed.queue_opt.queued));
				break;
			case queue_action::clear:
				clear(parsed.queue_opt.selected, assume_account(user).first);
				break;
			case queue_action::print:
				print_iterable(print(parsed.queue_opt.selected, assume_account(user).first));
				break;
			}
			break;
		case mode::gen:
		{ //notice the braces- this is a scope
			outgoing_post post(parsed.gen_opt.filename);
			post.parsed = std::move(parsed.gen_opt.post);
		}
		pl() << "Wrote post template to " << parsed.gen_opt.filename;
		break;
		case mode::sync:
			if (parsed.sync_opts.send)
			{
				send_posts send{ simple_post };
				send.retries = parsed.sync_opts.retries;
				if (user == nullptr)
					send.send_all();
				else
					send.send(user->first, *user->second.get_option(user_option::instance_url), *user->second.get_option(user_option::access_token));
			}
			break;
		case mode::help:
			break;
		default:
			pl() << "[option not implemented]";
		}
	}
	catch (const std::exception & e)
	{
		pl() << "An error occurred: " << e.what();
		pl() << "\nFor account: " << parsed.account;
	}

	pl() << '\n';

	plfile() << "--- msync finished normally ---\n";
}

template <typename T>
void uniqueify(T& toprint)
{
	const auto last = std::unique(toprint.begin(), toprint.end());
	toprint.erase(last, toprint.end());
}

std::pair<const std::string, user_options>& assume_account(std::pair<const std::string, user_options>* user)
{
	if (user == nullptr)
		throw msync_exception("Could not find a match [or an unambiguous match].");
	return *user;
}

void print_stringptr(const std::string* toprint)
{
	if (toprint == nullptr)
		pl() << "[not set]";
	else
		pl() << *toprint;
}

template <typename T>
void print_iterable(const T& vec)
{
	bool minushelp = false;
	for (auto& item : vec)
	{
		pl() << item << '\n';
		minushelp = minushelp || (item.back() == '-');
	}
	if (minushelp)
		pl() << "IDs followed by a - will be deleted next time you sync.\n";

}