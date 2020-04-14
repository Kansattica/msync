#include "executable_location.hpp"

#include <string>
#include <whereami.h>

fs::path get_executable_folder()
{
    const size_t length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::string(length + 1, '\0');

    int dirname_length;
    wai_getExecutablePath(&path[0], length, &dirname_length);
    return fs::path(path.begin(), path.begin() + dirname_length);
}

const fs::path& executable_folder()
{
    const static fs::path folder = get_executable_folder();
    return folder;
}
