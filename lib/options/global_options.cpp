#include "global_options.hpp"
#include "filesystem.hpp"
#include <constants.hpp>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <exception>
#include <whereami.h>

global_options options;

using namespace std::string_literals;

bool global_options::add_new_account(std::string name)
{
    print_logger<logtype::verbose> pl;
    fs::path user_path{executable_location};
    user_path /= Account_Directory;

    if (!fs::exists(user_path)) // ensure accounts directory exists
        fs::create_directory(user_path);

    if (!fs::is_directory(user_path))
        throw msync_exception("Path exists and is not a directory. Please rename or delete this file and try again: "s + user_path.string());

    user_path /= name;

    const auto [it, inserted] = accounts.emplace(std::move(name), user_options{user_path});

    if (!inserted)
        pl << "Account already exists. Nothing changed.\n";

    return true;
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
    print_logger<logtype::verbose> pl;
    pl << "Reading accounts from " << executable_location << "\n";

    std::unordered_map<std::string, user_options> toreturn;

    return toreturn;
}