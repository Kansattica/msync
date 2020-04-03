#include <exception>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <string>
#include <string_view>
#include <algorithm>

#include "version.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/options/option_enums.hpp"
#include "../lib/options/user_options.hpp"
#include "../lib/postfile/outgoing_post.hpp"
#include "../lib/queue/queues.hpp"
#include "../lib/sync/send.hpp"
#include "../lib/sync/recv.hpp"
#include "../lib/net/net.hpp"
#include "../lib/util/util.hpp"
#include "new_account.hpp"
#include "optionparsing/parse_options.hpp"

std::pair<const std::string, user_options>& assume_account(const std::string& account);
std::pair<const std::string, user_options>& assume_account(std::pair<const std::string, user_options>* user);

void do_sync(const parse_result& parsed);

void show_all_options(std::pair<const std::string, user_options>* user_ptr, const parse_result& parsed);

void print_stringptr(const std::string* toprint);
void print_sensitive(std::string_view name, const std::string* value);

template <typename T>
void print_iterable(const T& vec);

int main(int argc, const char* argv[])
{
	plfile() << "--- msync started ---\n";

	auto parsed = parse(argc, argv, false);

	bool should_print_newline = true;
	try
	{
		switch (parsed.selected)
		{
		case mode::newuser:
			make_new_account(parsed.account);
			break;
		case mode::showopt:
			print_stringptr(assume_account(parsed.account).second.try_get_option(parsed.toset));
			break;
		case mode::showallopt:
			should_print_newline = false;
			show_all_options(options().select_account(parsed.account), parsed);
			break;
		case mode::config:
			should_print_newline = false;
			assume_account(parsed.account).second.set_option(parsed.toset, parsed.optionval);
			break;
		case mode::configsync:
			should_print_newline = false;
			assume_account(parsed.account).second.set_option(parsed.toset, parsed.sync_opts.mode);
			break;
		case mode::configlist:
			should_print_newline = false;
			assume_account(parsed.account).second.set_option(parsed.toset, parsed.listops);
			break;
		case mode::queue:
			should_print_newline = false;
			switch (parsed.queue_opt.to_do)
			{
			case queue_action::add:
				enqueue(parsed.queue_opt.selected, assume_account(parsed.account).first, std::move(parsed.queue_opt.queued));
				break;
			case queue_action::remove:
				dequeue(parsed.queue_opt.selected, assume_account(parsed.account).first, std::move(parsed.queue_opt.queued));
				break;
			case queue_action::clear:
				clear(parsed.queue_opt.selected, assume_account(parsed.account).first);
				break;
			case queue_action::print:
				print_iterable(print(assume_account(parsed.account).first));
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
			should_print_newline = false;
			do_sync(parsed);
			break;
		case mode::help:
			should_print_newline = false;
			break;
		case mode::version:
			pl() << "msync version " << MSYNC_VERSION;
			break;
		case mode::yeehaw:
			plverb() << " __________\n"
						"<  yeehaw  >\n"
						" ----------\n"
						"  /\n";
			pl() << u8"🤠";
			break;
		default:
			pl() << "[option not implemented]";
		}
	}
	catch (const std::exception& e)
	{
		pl() << "An error occurred: " << e.what();
		pl() << "\nFor account: ";
		if (parsed.account.empty())
			pl() << "[default]";
		else
			pl() << parsed.account;
	}

	if (should_print_newline)
		pl() << '\n';

	plfile() << "--- msync finished normally ---\n";
}

void do_sync(const parse_result& parsed)
{
	auto user = options().select_account(parsed.account);
	if (parsed.sync_opts.send)
	{
		send_posts send{ simple_post, simple_delete, new_status, upload_media };
		send.retries = parsed.sync_opts.retries;
		if (user == nullptr)
		{
			options().foreach_account([&send](const auto& user) {
				send.send(user.first, user.second.get_option(user_option::instance_url), user.second.get_option(user_option::access_token)); });
		}
		else
			send.send(user->first, user->second.get_option(user_option::instance_url), user->second.get_option(user_option::access_token));
	}

	if (parsed.sync_opts.get)
	{
		recv_posts recv{ get_timeline_and_notifs };
		recv.max_requests = parsed.sync_opts.max_requests;
		recv.per_call = parsed.sync_opts.per_call;
		recv.retries = parsed.sync_opts.retries;

		if (user == nullptr)
		{
			options().foreach_account([&recv](auto& user) {
				recv.get(user.first, user.second); });
		}
		else
			recv.get(user->first, user->second);
	}
}

bool is_sensitive(user_option opt)
{
	for (const user_option sensitive : { user_option::access_token, user_option::auth_code, user_option::client_id, user_option::client_secret })
	{
		if (opt == sensitive)
			return true;
	}

	return false;
}

void show_all_options(std::pair<const std::string, user_options>* user_ptr, const parse_result& parsed)
{
	pl() << "Accounts registered:\n";
	{
		const auto accountnames = options().all_accounts();
		if (accountnames.empty())
		{
			pl() << "None. Run msync new --account [username@instance.url] to register an account with msync.\n";
			return;
		}
		std::for_each(accountnames.begin(), accountnames.end(), [](const auto& accountname) { pl() << accountname << '\n'; });
	}

	const auto& user = assume_account(user_ptr);
	for (auto opt = user_option(0); opt <= user_option::pull_notifications; opt = user_option(static_cast<int>(opt) + 1))
	{
		const auto option_name = USER_OPTION_NAMES[static_cast<int>(opt)];
		if (opt < user_option::exclude_follows)
		{
			const auto option_value = user.second.try_get_option(opt);
			if (is_sensitive(opt))
			{
				print_sensitive(option_name, option_value);
			}
			else
			{
				pl() << option_name << ": ";
				print_stringptr(option_value);
				pl() << '\n';
			}
		}
		else if (opt >= user_option::exclude_follows && opt <= user_option::exclude_polls)
		{
			pl() << option_name << ": " << (user.second.get_bool_option(opt) ? "true" : "false") << '\n';
		}
		else
		{
			pl() << option_name << ": " << SYNC_SETTING_NAMES[static_cast<int>(user.second.get_sync_option(opt))] << '\n';
		}
	}
}

std::pair<const std::string, user_options>& assume_account(std::pair<const std::string, user_options>* user)
{
	if (user == nullptr)
		throw msync_exception("Could not find a match [or an unambiguous match].");
	return *user;
}

std::pair<const std::string, user_options>& assume_account(const std::string& account)
{
	return assume_account(options().select_account(account));
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
	for (const auto& item : vec)
	{
		pl() << item << '\n';
		minushelp = minushelp || (item.back() == '-');
	}
	if (minushelp)
		pl() << "IDs followed by a - will be deleted next time you sync.\n";
}

void print_sensitive(std::string_view name, const std::string* value)
{
	if (value == nullptr)
		return;

	pl() << name << ": [set, not shown for security]\n";
}
