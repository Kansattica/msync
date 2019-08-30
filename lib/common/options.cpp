#include "options.hpp"

#include <constants.hpp>

#include <vector>

#ifndef __clang__
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

#include <whereami.h>

global_options options;

void set_locations()
{
    options.current_working_directory = std::filesystem::current_path();

    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(length + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    path[dirname_length + 1] = '\0';
    options.executable_location = std::filesystem::path(path.get());
}

std::optional<std::string> account_exists(std::string name)
{
    return Account_Directory;
}
