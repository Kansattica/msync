#include "global_options.hpp"
#include "user_options.hpp"
#include "../constants/constants.hpp"
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <algorithm>
#include <iterator>
#include <limits>

#include <cctype>

using idx_size_t = std::vector<std::pair<const std::string, user_options>>::size_type;
constexpr auto no_default_account = std::numeric_limits<idx_size_t>::max();

global_options::global_options(fs::path accounts_dir) : accounts_directory(std::move(accounts_dir)), default_account_idx(no_default_account)
{
	plverb() << "Reading accounts from " << accounts_directory << "\n";

	if (!fs::exists(accounts_directory))
		return;

	idx_size_t idx = 0;
	for (const auto& userfolder : fs::directory_iterator(accounts_directory))
	{
		if (!fs::is_directory(userfolder.path()))
		{
			plverb() << userfolder.path() << " is not a directory. Skipping.\n";
			continue;
		}

		fs::path configfile = userfolder.path() / User_Options_Filename;

		if (!fs::exists(configfile))
		{
			using namespace std::string_literals;
			throw msync_exception("Expected to find a config file and didn't find it. Try deleting the folder and running new again: "s + to_utf8(userfolder.path()));
		}

		auto& inserted = accounts.emplace_back(to_utf8(userfolder.path().filename()), user_options{ std::move(configfile) });
		if (inserted.second.get_bool_option(user_option::is_default))
			default_account_idx = idx;
		idx++;
	}
}

std::pair<const std::string, user_options>& global_options::add_new_account(std::string name)
{
	const auto contains = std::find_if(accounts.begin(), accounts.end(), [&name](const auto& account_pair) { return name == account_pair.first; });
	if (contains != accounts.end())
	{
		plverb() << "Account " << name << " already exists.";
		return *contains;
	}

	fs::path user_path = accounts_directory / name;

	fs::create_directories(user_path); //can throw if something goes wrong
	fs::permissions(accounts_directory, fs::perms::owner_all);

	user_path /= User_Options_Filename;

	return accounts.emplace_back(std::move(name), user_options{ std::move(user_path) });
}

select_account_result global_options::select_account(std::string_view name)
{
	if (accounts.empty()) { return select_account_error::no_accounts; }

	if (!name.empty() && name.front() == '@') { name.remove_prefix(1); } //remove leading @s

	if (name.empty() && default_account_idx != -1)
	{
		plverb() << "Matched default account " << accounts[default_account_idx].first << '\n';
		return &accounts[default_account_idx];
	}

	std::pair<const std::string, user_options>* candidate = nullptr;

	for (auto& entry : accounts)
	{
		// if name is longer than the entry, we'll step off the end of entry and segfault
		// since name can't possibly match something it's longer than, just skip this
		if (name.size() > entry.first.size())
			continue;

		// won't have string.starts_with until c++20, so
		// if the name given is a prefix of (or equal to) this entry, it's a candidate
		// unsigned char because https://en.cppreference.com/w/cpp/string/byte/tolower
		// also, account names SHOULD only be ASCII, otherwise tolower is weird and
		// it might not only work. 
		if (std::equal(name.begin(), name.end(), entry.first.begin(), [](unsigned char a, unsigned char b) {
				return std::tolower(a) == std::tolower(b); //case insensitive
			}))
		{
			plverb() << "Matched account " << entry.first << '\n';

			// if this is the second candidate we've found, it's ambiguous and return nothing
			if (candidate != nullptr)
			{
				if (name.empty()) { return select_account_error::empty_name_many_accounts; }
				pl() << name << " could match either " << candidate->first << " or " << entry.first << ". Please specify an unambiguous prefix.\n";
				return select_account_error::ambiguous_prefix;
			}

			candidate = &entry;
		}
	}
	
	// nullptr if we found nothing, points to the account entry if we found exactly one candidate
	if (candidate == nullptr) { return select_account_error::bad_prefix; }
	return candidate;
}

select_account_result global_options::set_default(const std::string_view name)
{
	if (name.empty())
	{
		if (default_account_idx != no_default_account)
			accounts[default_account_idx].second.set_bool_option(user_option::is_default, false);
		default_account_idx = no_default_account;
		return nullptr;
	}

	auto selected = select_account(name);
	if (std::holds_alternative<select_account_error>(selected))
	{
		return selected;
	}

	if (default_account_idx != no_default_account)
	{
		accounts[default_account_idx].second.set_bool_option(user_option::is_default, false);
	}
	std::get<0>(selected)->second.set_bool_option(user_option::is_default, true);

	// subtract the selected pointer from the start of the accounts vector to get the index
	default_account_idx = std::get<0>(selected) - accounts.data();
	return selected;
}

std::vector<std::string_view> global_options::all_accounts() const
{
	std::vector<std::string_view> toreturn;
	toreturn.reserve(accounts.size());
	std::transform(accounts.begin(), accounts.end(), std::back_insert_iterator(toreturn), [](const auto& pair)
		{
			return std::string_view{ pair.first };
		});
	return toreturn;
}
