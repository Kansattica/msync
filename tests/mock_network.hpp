#ifndef _MOCK_NETWORK_HPP_
#define _MOCK_NETWORK_HPP_

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

#endif
