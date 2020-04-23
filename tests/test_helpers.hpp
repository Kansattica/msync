#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <filesystem.hpp>

#include <string>
#include <vector>
#include <string_view>

std::vector<std::string> read_lines(const fs::path& toread);
size_t count_files_in_directory(const fs::path& tocheck);
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
	test_file(fs::path name) : filename(std::move(name)), filenamebak(make_backup_if_not_directory())
	{
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
	const fs::path filenamebak;
private:

	fs::path make_backup_if_not_directory()
	{
		if (fs::is_directory(filename))
			return {};
		return fs::path{ filename }.concat(".bak");
	}
};

test_file temporary_file();

struct test_dir
{
public:
	test_dir(fs::path name) : dirname(std::move(name))
	{
		fs::remove_all(dirname);
		fs::create_directories(dirname);
	}

	~test_dir()
	{
		fs::remove_all(dirname);
	}

	test_dir(const test_dir&) = delete;

	const fs::path dirname;
};

test_dir temporary_directory();
#endif
