#ifndef _FILE_BACKED_HPP_
#define _FILE_BACKED_HPP_

#include <fstream>

#include <filesystem.hpp>
#include <print_logger.hpp>

template <typename Container, void(*Read)(Container&, std::string&&), void(*Write)(Container&&, std::ofstream&)>
class file_backed
{
public:
    Container parsed;
	file_backed(fs::path filename) : backing(filename)
	{
		print_logger<logtype::verbose> logger;
		std::ifstream backingfile(backing);
		for (std::string line; getline(backingfile, line);)
		{
			logger << "Parsing line: " << line << '\n';

			const auto first_non_whitespace = line.find_first_not_of(" \t\r\n");

			if (first_non_whitespace == std::string::npos)
				continue; // blank line?

			if (line[first_non_whitespace] == '#')
				continue; //skip comments

			Read(parsed, std::move(line));
		}
	}

	~file_backed()
	{
		if (backing == "")
			return; // we got moved from, so the new version will save it

		print_logger<logtype::verbose> logger;

		fs::path backup(backing);
		backup += ".bak";

		if (fs::exists(backing))
		{
			fs::rename(backing, backup);
			logger << "Saved backup to " << backup << '\n';
		}

		ofstream of(backing);
		Write(std::move(parsed), of);

		logger << "Saved " << backing << '\n';
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
