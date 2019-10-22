#ifndef _TEST_HELPERS_HPP_
#define _TEST_HELPERS_HPP_

#include <filesystem.hpp>

#include <string>
#include <vector>
#include <string_view>

std::vector<std::string> read_lines(const fs::path& toread);

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
	test_file(const char* name) : test_file(fs::path(name)) {};
	test_file(std::string_view name) : test_file(fs::path(name)) {};
	test_file(fs::path name) : filename(name)
	{
		if (!fs::is_directory(name))
			filenamebak = fs::path{ name }.concat(".bak");

		fs::remove_all(filename);

		if (!filenamebak.empty())
			fs::remove(filenamebak);
	};

	~test_file()
	{
		fs::remove_all(filename);

		if (!filenamebak.empty())
			fs::remove(filenamebak);
	};

	operator std::string() const { return filename.string(); }
	const fs::path filename;
	fs::path filenamebak;
private:
};

test_file account_directory();
#endif
