#include "global_options.hpp"
#include "filesystem.hpp"
#include <constants.hpp>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <whereami.h>

global_options options;

using namespace std::string_literals;

user_options& global_options::add_new_account(std::string name)
{
    print_logger<logtype::verbose> pl;
    fs::path user_path{executable_location};
    user_path /= Account_Directory;
    user_path /= name;

    fs::create_directories(user_path); //can throw if something goes wrong

    user_path /= User_Options_Filename;

    const auto [it, inserted] = accounts.emplace(std::move(name), user_options{user_path});

    if (!inserted)
        pl << "Account already exists. Nothing changed.\n";

    return it->second;
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