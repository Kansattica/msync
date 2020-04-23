#ifndef _POST_LIST_HPP_
#define _POST_LIST_HPP_

#include <filesystem.hpp>

#include <fstream>

#include "../entities/entities.hpp"

std::ostream& operator<<(std::ostream& out, const mastodon_status& status);
std::ostream& operator<<(std::ostream& out, const mastodon_notification& notification);
std::ostream& operator<<(std::ostream& out, const mastodon_poll& poll);

// currently supposed to work with statuses and notifications
template <typename post_type>
class post_list
{
public:

	// ofstream doesn't know what to do with Boost's filesystem paths, so call c_str()
	// this is harmless with non-Boost filesystems because those just turn around and call .c_str() on the path anyway
	post_list(const fs::path& filename) : outfile(filename.c_str(), std::ios::app | std::ios::ate | std::ios::out)
	{
	}

	void write(const post_type& post)
	{
		// this assumes that the operator<< for post doesn't add a newline at the end
		outfile << post;
		outfile << "\n--------------\n";
	}

private:
	std::ofstream outfile;
};
#endif
