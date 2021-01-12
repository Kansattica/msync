#ifndef SYNC_TEST_COMMON_HPP
#define SYNC_TEST_COMMON_HPP

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
