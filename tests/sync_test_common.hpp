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

inline void verify_file(const fs::path& file, int expected_count, const std::string& id_starts_with)
{
	static constexpr std::string_view dashes = "--------------";

	const auto lines = read_lines(file);

	bool read_next = true;
	unsigned int last_id = 0;
	unsigned int total = 0;

	for (const auto& line : lines)
	{
		if (line == dashes)
		{
			read_next = true;
			continue;
		}

		if (read_next)
		{
			read_next = false;
			REQUIRE_THAT(line, Catch::StartsWith(id_starts_with));

			unsigned int this_id;
			std::from_chars(line.data() + id_starts_with.size(), line.data() + line.size(), this_id);

			REQUIRE(this_id > last_id);

			last_id = this_id;
			total++;
		}
	}

	REQUIRE(expected_count == total);
}
#endif
