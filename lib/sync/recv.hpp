#ifndef _MSYNC_RECV_HPP_
#define _MSYNC_RECV_HPP_

#include <print_logger.hpp>

#include "../net_interface/net_interface.hpp"

#include <string_view>

template <typename get_posts>
struct recv_posts
{
public:
	unsigned int retries = 3;
	unsigned int max_requests = 10;
	unsigned int per_call = 20;

	recv_posts(get_posts& get_posts) : get(get_posts) {};

	void get(const std::string_view account)
	{
		retries = set_default(retries, 3, "Number of retries cannot be zero or less. Resetting to 3.\n", pl());
		max_requests = set_default(max_requests, 5, "Maximum requests cannot be zero or less. Resetting to 5.\n", pl());
		per_call = set_default(per_call, 20, "Number of posts to get per call cannot be zero or less. Resetting to 20.\n", pl());

		pl() << "Downloading notifications for " << account << '\n';
		pl() << "Downloading the home timeline for " << account << '\n';
	}

private:

	get_posts& get;
};

#endif