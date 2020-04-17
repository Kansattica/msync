#include "account_directory.hpp"

#include <constants.hpp>

#ifdef __linux__
#include <limits.h>
#include <cstdlib>
#include <memory>
#else
#include <string>
#include <whereami.h>
#endif
#ifdef USE_XDG_DIRECTORY_SPEC
#include <cstdlib>
#endif
fs::path get_executable_folder()
{
#ifdef USE_XDG_DIRECTORY_SPEC
	fs::path to_return;
	char *env_value=nullptr;
	// If this environmental variable is set it overrides the usual ~/.config/<stuff> paths
	if((env_value = getenv("XDG_CONFIG_HOME")) != nullptr)
	{
		to_return = env_value;
	}
	else
	{
		to_return = getenv("HOME");
		to_return/=".config";
	}
	to_return/="msync";
	return to_return;
	// whereami doesn't work on arm processors for some reason, and also whereami uses PATH_MAX on Linux which you shouldn't do
	// (see https://linux.die.net/man/3/realpath, stackoverflow answers, and blogs that say you shouldn't use PATH_MAX)
	// so use the correct form of realpath on linux and whereami everywhere else
#elif defined(__linux__)
	// this version of realpath malloc()s a buffer and returns it, so we use unique_ptr to free it automatically.
	std::unique_ptr<char[], decltype(std::free)*> full_executable_path { realpath("/proc/self/exe", NULL), std::free };
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


const fs::path& account_directory_path()
{
	const static fs::path folder = get_executable_folder().append(Account_Directory);
	return folder;
}
