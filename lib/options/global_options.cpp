#include "global_options.hpp"
#include "user_options.hpp"
#include "../constants/constants.hpp"
#include "../executablelocation/executable_location.hpp"
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <algorithm>
#include <iterator>

#include <cctype>

global_options& options()
{
	static global_options options;
	return options;
}

global_options::global_options() : account_directory_location(executable_folder() / Account_Directory)
{
	plverb() << "Reading accounts from " << account_directory_location << "\n";

	if (!fs::exists(account_directory_location))
		return;

	for (const auto& userfolder : fs::directory_iterator(account_directory_location))
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
			throw msync_exception("Expected to find a config file and didn't find it. Try deleting the folder and running new again: "s + userfolder.path().string());
		}

		accounts.emplace_back(userfolder.path().filename().string(), user_options{ std::move(configfile) });
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

    fs::path user_path = account_directory_location / name;

    fs::create_directories(user_path); //can throw if something goes wrong

    user_path /= User_Options_Filename;

	return accounts.emplace_back(std::move(name), user_options{ std::move(user_path) });
}

std::pair<const std::string, user_options>* global_options::select_account(std::string_view name)
{
	if (!name.empty() && name.front() == '@') { name.remove_prefix(1); } //remove leading @s

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
        if (std::equal(name.begin(), name.end(), entry.first.begin(), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b); //case insensitive
            }))
        {
            plverb() << "Matched account " << entry.first << '\n';

			// if this is the second candidate we've found, it's ambiguous and return nothing
			if (candidate != nullptr) { return nullptr; }

            candidate = &entry;
        }
    }
	
	// nullptr if we found nothing, points to the account entry if we found exactly one candidate
	return candidate;
}

void global_options::clear_accounts()
{
	std::for_each(accounts.begin(), accounts.end(), 
		[this](const auto& account) { fs::remove_all(account_directory_location / account.first); });

	accounts.clear();
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
