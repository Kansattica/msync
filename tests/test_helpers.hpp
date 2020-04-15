#ifndef _TEST_HELPERS_HPP_
#define _TEST_HELPERS_HPP_

#include <filesystem.hpp>

#include <string>
#include <vector>
#include <string_view>

std::vector<std::string> read_lines(const fs::path& toread);
size_t count_files_in_directory(const fs::path& tocheck);
void touch(const fs::path& totouch);
std::string read_file(const fs::path& file);

void make_status_json(std::string_view id, std::string& to_append);
void make_notification_json(std::string_view id, std::string& to_append);

std::vector<std::string> make_expected_ids(const std::vector<std::string>& ids, std::string_view prefix);

bool flip_coin();
int zero_to_n(int n);

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
	test_file(const char* name) : test_file(fs::path(name)) {};
	test_file(std::string_view name) : test_file(fs::path(name.begin(), name.end())) {}; //use iterator interface for boost's benefit
	test_file(fs::path name) : filename(std::move(name))
	{
		if (!fs::is_directory(filename))
			filenamebak = fs::path{ filename }.concat(".bak");

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

	operator const fs::path::value_type* () const { return filename.c_str(); }
	const fs::path filename;
	fs::path filenamebak;
private:
};

//ensures a file only exists during each test run
struct touch_file
{
public:
	touch_file(const char* name) : touch_file(fs::path(name)) {};
	touch_file(std::string_view name) : touch_file(fs::path(name.begin(), name.end())) {};
	touch_file(fs::path name) : filename(std::move(name))
	{
		if (!fs::exists(filename))
			touch(filename);
	};

	~touch_file()
	{
		fs::remove(filename);
	};

	operator const fs::path::value_type* () const { return filename.c_str(); }
	const fs::path filename;
};

test_file clean_account_directory();
#endif
