#ifndef _POST_LIST_HPP_
#define _POST_LIST_HPP_

#include <filesystem.hpp>

#include <vector>
#include <fstream>

#include "../entities/entities.hpp"

// these should be ostreams, but I didn't want to pull in the whole iostream library just for this
std::ofstream& operator<<(std::ofstream& out, const mastodon_status& status);
std::ofstream& operator<<(std::ofstream& out, const mastodon_notification& notification);

// currently supposed to work with statuses and notifications
template <typename post_type>
class post_list
{
public:
    std::vector<post_type> toappend;
	post_list(fs::path filename) : backing(filename)
	{
	}

	~post_list()
	{
		if (backing.empty()) { return; } // if we got moved from, don't touch the file in this one

		std::ofstream of(backing, std::ios::app | std::ios::ate | std::ios::out);
		for (const auto& post : toappend)
		{
			// this assumes that the operator<< for post adds a newline at the end
			of << post;
			of << "--------------\n";
		}
	}

	// can be moved
	post_list(post_list&& other) noexcept // move constructor
		: backing(std::move(other.backing)), toappend(std::move(other.toappend))
	{
	}

	post_list& operator=(post_list&& other) noexcept // move assignment
	{
		std::swap(toappend, other.toappend);
		std::swap(backing, other.backing);
		return *this;
	}

	//  can't be copied
	post_list(const post_list& other) = delete;            // copy constructor
	post_list& operator=(const post_list& other) = delete; // copy assignment

private:
	fs::path backing;
};
#endif
