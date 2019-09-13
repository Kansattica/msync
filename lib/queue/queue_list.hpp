#ifndef _QUEUE_LIST_HPP_
#define _QUEUE_LIST_HPP_

#include <filesystem.hpp>
#include <deque>
#include <string>

using std::string;

struct queue_list
{
public:
    std::deque<string> queued;
    queue_list(fs::path filename);
    ~queue_list();

    // can be moved
    queue_list(queue_list&& other) noexcept // move constructor
        : listfilename(std::move(other.listfilename)), queued(std::move(other.queued))
    {
    }

    queue_list& operator=(queue_list&& other) noexcept // move assignment
    {
        std::swap(queued, other.queued);
        std::swap(listfilename, other.listfilename);
        return *this;
    }

    // optionfiles can't be copied
    queue_list(const queue_list& other) = delete;            // copy constructor
    queue_list& operator=(const queue_list& other) = delete; // copy assignment

private:
    fs::path listfilename;
};

#endif
