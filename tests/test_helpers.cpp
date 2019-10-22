#include "test_helpers.hpp"

#include <memory>
#include <whereami.h>

#include <fstream>

fs::path _get_exe_location()
{
    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(static_cast<size_t>(length) + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    return fs::path(path.get(), path.get() + dirname_length);
}

const fs::path _exeloc = _get_exe_location();
const fs::path& exe_location()
{
	return _exeloc;
}

std::vector<std::string> read_lines(fs::path toread)
{
	std::ifstream fin(toread);
	std::vector<std::string> toreturn;

	for (std::string line; std::getline(fin, line);)
	{
		toreturn.push_back(line);
	}

	return toreturn;
}
