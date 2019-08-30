#include "options.hpp"

#include <constants.hpp>

#include <vector>

#ifdef __cpp_lib_filesystem
#include <filesystem>
using fs = std::filesystem;
#warning "Regular filesystem found."
#elif __cpp_lib_experimental_filesystem
#include <experimental/filesystem>
using fs = std::experimental::filesystem;
#warning "Experimental filesystem found."
#else
#error "no filesystem support ='("
#endif

#include <whereami.h>

global_options options;

void set_locations()
{
    options.current_working_directory = fs::current_path();

    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(length + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    path[dirname_length + 1] = '\0';
    options.executable_location = fs::path(path.get());
}

std::optional<std::string> account_exists(std::string name)
{
    return Account_Directory;
}
