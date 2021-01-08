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

template <typename make_object>
std::string make_json_array(make_object func, unsigned int min_id, unsigned int max_id)
{
	std::array<char, 10> char_buf;

	std::string toreturn(1, '[');

	// basically, it shouldn't return max_id or min_id itself
	// and the newest (highest ID) goes first
	for (unsigned int id = max_id; id > min_id; id--)
	{
		func(sv_to_chars(id, char_buf), toreturn);
		toreturn.append(1, ',');
	}
	toreturn.pop_back(); //get rid of that last comma
	if (!toreturn.empty())
		toreturn += ']';
	return toreturn;
}

inline std::vector<std::string> copy_excludes(std::vector<std::string_view>* ex)
{
	if (ex == nullptr) { return {}; }

	return std::vector<std::string>(ex->begin(), ex->end());
}
#endif
