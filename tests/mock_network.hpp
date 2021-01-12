#ifndef MOCK_NETWORK_HPP
#define MOCK_NETWORK_HPP

#include "to_chars_patch.hpp"

struct mock_network
{
	int status_code = 200;
	bool fatal_error = false;

	void set_succeed_after(size_t n)
	{
		succeed_after = succeed_after_n = n;
	}

protected:
	size_t succeed_after_n = 1;
	size_t succeed_after = succeed_after_n;
};

struct basic_mock_args
{
	unsigned int sequence;
	std::string url;
	std::string access_token;
};

struct get_mock_args : basic_mock_args
{
	std::string min_id;
	std::string max_id;
	std::string since_id;
	std::vector<std::string> exclude_notifs;
	unsigned int limit;
};

#endif
