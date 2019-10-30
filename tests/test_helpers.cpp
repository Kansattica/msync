#include "test_helpers.hpp"

#include <memory>
#include <whereami.h>

#include <fstream>

#include "../lib/constants/constants.hpp"

fs::path _get_exe_location()
{
    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(static_cast<size_t>(length) + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    return fs::path(path.get(), path.get() + dirname_length);
}

const static fs::path _accountdir = _get_exe_location() / Account_Directory;
test_file account_directory()
{
	return test_file{ _accountdir };
}

std::vector<std::string> read_lines(const fs::path& toread)
{
	std::ifstream fin(toread);
	std::vector<std::string> toreturn;

	for (std::string line; std::getline(fin, line);)
	{
		toreturn.push_back(std::move(line));
	}

	return toreturn;
}

size_t count_files_in_directory(const fs::path& tocheck)
{
	if (!fs::is_directory(tocheck)) { return false; }

	size_t count = 0;
	auto dir = fs::directory_iterator(tocheck);
	while (dir != fs::end(dir))
	{
		++count;
		++dir;
	}
	return count;
}

void touch(const fs::path& totouch)
{
	std::ofstream of(totouch, std::ios::out | std::ios::app);
}
