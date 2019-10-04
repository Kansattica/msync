#include "global_options.hpp"
#include "user_options.hpp"
#include <constants.hpp>
#include <filesystem.hpp>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <whereami.h>

#include <cctype>

global_options options;

using namespace std::string_literals;

std::pair<const std::string, user_options>& global_options::add_new_account(std::string name)
{
    fs::path user_path = executable_location;
    user_path /= Account_Directory;
    user_path /= name;

    fs::create_directories(user_path); //can throw if something goes wrong

    user_path /= User_Options_Filename;

    const auto [it, inserted] = accounts.emplace(name, user_options{user_path});

    if (!inserted)
        plverb() << "Account already exists. Nothing changed.\n";

    return *it;
}

fs::path global_options::get_exe_location()
{
    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(length + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    return fs::path(path.get(), path.get() + dirname_length);
}

std::unordered_map<std::string, user_options> global_options::read_accounts()
{
    fs::path account_location = executable_location / Account_Directory;

    plverb() << "Reading accounts from " << account_location << "\n";

    std::unordered_map<std::string, user_options> toreturn;

    if (!fs::exists(account_location))
        return toreturn;

    for (auto& userfolder : fs::directory_iterator(account_location))
    {
        if (!fs::is_directory(userfolder))
        {
            plverb() << userfolder << " is not a directory. Skipping.\n";
            continue;
        }

        fs::path configfile = userfolder / User_Options_Filename;

        if (!fs::exists(configfile))
        {
            throw msync_exception("Expected to find a config file and didn't find it. Try deleting the folder and running new again: " + userfolder.path().string());
        }

        auto accountname = userfolder.path().filename().string();
        user_options config_file{configfile};

        toreturn.emplace(accountname, std::move(config_file));
    }

    return toreturn;
}

std::pair<const std::string, user_options>* global_options::select_account(const std::string_view name)
{
    int matched = 0;
	std::pair<const std::string, user_options>* candidate = nullptr;

    for (auto& entry : accounts)
    {

        // if name is longer than the entry, we'll step off the end of entry and segfault
        // since name can't possibly match something it's longer than, just skip this
        if (name.size() > entry.first.size())
            continue;

        // won't have string.starts_with until c++20, so
        // if the name given is a prefix of (or equal to) this entry, it's a candidate
        if (std::equal(name.begin(), name.end(), entry.first.begin(), [](auto a, auto b) {
                return std::tolower(a) == std::tolower(b); //case insensitive
            }))
        {
            plverb() << "Matched account" << entry.first << "\n";
            matched++;
            candidate = &entry;
        }
    }

    if (matched == 1)
        return candidate;

    return nullptr;
}