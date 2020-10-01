#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <filesystem.hpp>

#include <string>
#include <vector>
#include <string_view>
#include <time.h> //ctime doesn't have gmtime_s

std::vector<std::string> read_lines(const fs::path& toread);
size_t count_files_in_directory(const fs::path& tocheck);
std::string read_file(const fs::path& file);

void make_status_json(std::string_view id, std::string& to_append);
void make_notification_json(std::string_view id, std::string& to_append);

std::vector<std::string> make_expected_ids(const std::vector<std::string>& ids, std::string_view prefix);

bool flip_coin();
int zero_to_n(int n);

void wrap_gmtime(struct tm* utctime, const time_t* since_epoch);

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
	test_file(const char* name) : test_file(fs::path(name)) {};
	test_file(std::string_view name) : test_file(fs::path(name.begin(), name.end())) {}; //use iterator interface for boost's benefit
	test_file(fs::path name) : _filename(std::move(name)), _filenamebak(make_backup_if_not_directory())
	{
		fs::remove_all(_filename);

		if (!_filenamebak.empty())
			fs::remove(_filenamebak);
	};

	~test_file()
	{
		if (_filename.empty()) { return; }

		fs::remove_all(_filename);

		if (!_filenamebak.empty())
			fs::remove(_filenamebak);
	};

	// can be moved
	test_file(test_file&& other) noexcept // move constructor
		:  _filename(std::move(other._filename)), _filenamebak(std::move(other._filenamebak))
	{
		other._filename.clear();
	}

	test_file& operator=(test_file&& other) noexcept // move assignment
	{
		std::swap(_filename, other._filename);
		std::swap(_filenamebak, other._filenamebak);
		return *this;
	}

	test_file(test_file& other) = delete; // copy constructor

	operator const fs::path::value_type* () const noexcept { return _filename.c_str(); }

	const fs::path& filename() const noexcept { return _filename; }
	const fs::path& filenamebak() const noexcept { return _filenamebak; }

private:

	fs::path _filename;
	fs::path _filenamebak;

	fs::path make_backup_if_not_directory()
	{
		if (fs::is_directory(_filename))
			return {};
		return fs::path{ _filename }.concat(".bak");
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
