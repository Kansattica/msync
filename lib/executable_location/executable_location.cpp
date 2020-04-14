#include "executable_location.hpp"

#include <string>
#include <whereami.h>

#ifdef __linux__
#include <limits.h>
#include <stdlib.h>
#include <memory>
#endif

fs::path get_fallback_executable_folder()
{

	// whereami doesn't work on arm processors for some reason, so this is a fallback
#ifdef __linux__
	// see https://linux.die.net/man/3/realpath
	std::unique_ptr<char[]> full_executable_path { realpath("/proc/self/exe", NULL) };
	fs::path to_return { full_executable_path.get() };
	to_return.remove_filename();
	return to_return;
#else
	return fs::current_path(); //probably wrong, but it's an alright fallback? Might be better to just scream and die.
#endif
}

fs::path get_executable_folder()
{
    const int length = wai_getModulePath(nullptr, 0, nullptr);

	if (length <= 0)
		return get_fallback_executable_folder();

    auto path = std::string((size_t)length + 1, '\0');

    int dirname_length;
    wai_getExecutablePath(&path[0], length, &dirname_length);
    return fs::path(path.begin(), path.begin() + dirname_length);
}

const fs::path& executable_folder()
{
    const static fs::path folder = get_executable_folder();
    return folder;
}
