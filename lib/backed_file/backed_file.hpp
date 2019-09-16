#ifndef _BACKED_FILE_HPP_
#define _BACKED_FILE_HPP_

#include <filesystem.hpp>
#include <fstream>
#include <string>

template <typename Container, typename void Read(std::string&&, Container&), typename void Write(Container&&, std::ofstream&)>
class backed_file
{
public:
    Container backed;
    backed_file(fs::path filename);
    ~backed_file();

    // can be moved
    backed_file(backed_file&& other) noexcept // move constructor
        : backedfilename(std::move(other.backedfilename)), backed(std::move(other.backed))
    {
    }

    backed_file& operator=(backed_file&& other) noexcept // move assignment
    {
        std::swap(backed, other.backed);
        std::swap(backedfilename, other.backedfilename);
        return *this;
    }

    // backedfiles can't be copied
    backed_file(const backed_file& other) = delete;            // copy constructor
    backed_file& operator=(const backed_file& other) = delete; // copy assignment

private:
    fs::path backedfilename;
};

template <typename Container, typename void Read(std::string&&, Container&), typename void Write(Container&&, std::ofstream&)>
backed_file<Container, Read, Write>::backed_file(fs::path filename) : backedfilename(filename)
{
    std::ifstream backedfile(backedfilename);
    for (std::string line; std::getline(backedfile, line);)
    {
        const auto first_non_whitespace = line.find_first_not_of(" \t\r\n");

        if (first_non_whitespace == std::string::npos)
            continue; // blank line?

        if (line[first_non_whitespace] == '#')
            continue; //skip comments

        Read(std::move(line), backed);
    }
}

template <typename Container, typename void Read(std::string&&, Container&), typename void Write(Container&&, std::ofstream&)>
backed_file<Container, Read, Write>::~backed_file()
{
    if (backedfilename == "")
        return; // backedfile got moved from, so the new version will save it

    fs::path backup{backedfilename};
    backup += ".bak";

    if (fs::exists(backedfilename))
    {
        fs::rename(backedfilename, backup);
    }

    std::ofstream of(backedfilename);
    Write(std::move(backed), of);
}
#endif