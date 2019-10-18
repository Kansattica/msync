#ifndef _TEST_HELPERS_HPP_
#define _TEST_HELPERS_HPP_

#include <filesystem.hpp>

#include <string>
#include <vector>
#include <string_view>

const fs::path& exe_location();
std::vector<std::string> read_lines(fs::path toread);

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
	test_file(const char* name) : test_file(fs::path(name)) {};
	test_file(const std::string_view name) : test_file(fs::path(name)) {};
	test_file(fs::path name) : filename(name), filenamebak(fs::path{ name }.concat(".bak"))
	{
		fs::remove_all(filename);
		fs::remove(filenamebak);
	};

	~test_file()
	{
		fs::remove_all(filename);
		fs::remove(filenamebak);
	};

	operator std::string() const { return filename.string(); }
	const fs::path filename;
private:
	const fs::path filenamebak;
};

#endif
