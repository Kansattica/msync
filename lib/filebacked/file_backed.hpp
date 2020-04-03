#ifndef _FILE_BACKED_HPP_
#define _FILE_BACKED_HPP_

#include <fstream>

#include <iterator>

#include <filesystem.hpp>

template <typename Container, bool(*Read)(Container&, std::string&&), void(*Write)(Container&&, std::ofstream&), bool skip_blank = true, bool skip_comment = true, bool read_only = false>
class file_backed
{
public:
    Container parsed;

	file_backed(fs::path filename) : backing(filename)
	{
#ifdef MSYNC_USE_BOOST
		std::ifstream backingfile(backing.native());
#else
		std::ifstream backingfile(backing);
#endif
		for (std::string line; getline(backingfile, line);)
		{
			const auto first_non_whitespace = line.find_first_not_of(" \t\r\n");

			if constexpr (skip_blank)
				if (first_non_whitespace == std::string::npos)
					continue; // blank line?

			if constexpr (skip_comment)
				if (line[first_non_whitespace] == '#')
					continue; //skip comments

			// if they return true...
			if (Read(parsed, std::move(line)))
			{
				// ...read to end of string https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
				Read(parsed, std::string(std::istreambuf_iterator<char>(backingfile), {}));
			}
		}
	}

	~file_backed()
	{
		if constexpr (read_only)
		{
			return;
			// if they only wanted to look at the thing, don't save the changes
		}

		if (backing.empty())
			return; // we got moved from, so the new version will save it

		if (fs::exists(backing))
		{
			// gotta make a copy here
			const fs::path backup = fs::path{ backing }.concat(".bak");
#ifdef _WIN32
			// sometimes, Windows doesn't do the rename correctly and throws an "access denied" error when
			// renaming over an existing file.
			fs::remove(backup);
#endif
			fs::rename(backing, backup);
		}

#ifdef MSYNC_USE_BOOST
		std::ofstream of{ backing.native() };
#else
		std::ofstream of{ backing };
#endif
		Write(std::move(parsed), of);
	}

	// can be moved
	file_backed(file_backed&& other) noexcept // move constructor
		: backing(std::move(other.backing)), parsed(std::move(other.parsed))
	{
	}

	file_backed& operator=(file_backed&& other) noexcept // move assignment
	{
		std::swap(parsed, other.parsed);
		std::swap(backing, other.backing);
		return *this;
	}

	// backedfiles can't be copied
	file_backed(const file_backed& other) = delete;            // copy constructor
	file_backed& operator=(const file_backed& other) = delete; // copy assignment

private:
	fs::path backing;
};

#endif
