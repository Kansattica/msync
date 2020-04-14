#include "executable_location.hpp"


#ifdef __linux__
#include <limits.h>
#include <stdlib.h>
#include <memory>
#else
#include <string>
#include <whereami.h>
#endif

fs::path get_executable_folder()
{
	// whereami doesn't work on arm processors for some reason, and also whereami uses PATH_MAX on Linux which you shouldn't do
	// (see https://linux.die.net/man/3/realpath, stackoverflow answers, and blogs that say you shouldn't use PATH_MAX)
    // so use the correct form of realpath on linux and whereami everywhere else
#ifdef __linux__

    // this version of realpath malloc()s a buffer and returns it, so we use unique_ptr to free it automatically.
	std::unique_ptr<char[]> full_executable_path { realpath("/proc/self/exe", NULL) };
	fs::path to_return { full_executable_path.get() };
	to_return.remove_filename();
	return to_return;
#else
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::string((size_t)length + 1, '\0');

    int dirname_length;
    wai_getExecutablePath(&path[0], length, &dirname_length);
    return fs::path(path.begin(), path.begin() + dirname_length);
#endif
}


const fs::path& executable_folder()
{
    const static fs::path folder = get_executable_folder();
    return folder;
}
