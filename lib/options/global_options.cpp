#include "global_options.hpp"

#include <print_logger.hpp>
#include <whereami.h>

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