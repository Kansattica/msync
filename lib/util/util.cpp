#include "util.hpp"

#include <regex>

std::string make_api_url(const std::string_view instance_url, const std::string_view api_route)
{
	std::string to_return{ "https://" };
	to_return.reserve(instance_url.size() + api_route.size() + to_return.size());
	to_return.append(instance_url).append(api_route);
	return to_return;
}

std::optional<parsed_account> parse_account_name(const std::string& name)
{
	const static std::regex account_name{ R"(@?([-_~a-z0-9]+)@(?:https?://)?([-_~a-z0-9-]+\.[-_~a-z0-9-]+(?:\.[-_~a-z0-9-]+)?)[, =/\\?]*$)", std::regex::ECMAScript | std::regex::icase };

	std::smatch results;
	if (std::regex_match(name, results, account_name))
	{
		return parsed_account{ results[1], results[2] };
	}

	return {};
}

// if src is null, modifies dest in place
extern "C" size_t decode_html_entities_utf8(char* dest, const char* src);

std::string clean_up_html(const std::string_view to_strip)
{
	if (to_strip.empty()) { return {}; }

	const static std::regex remove_tags{ "<[^<]*>" };
	const static std::regex replace_line_breaks{ "<br */?>" };
	const static std::regex replace_paragraph_breaks{ "</p>\\s*<p>" };

	// regex_replace will always keep the string the same length or make it shorter, so it won't cause an overflow
	// the author of decode_html_entities says the same thing
	// I think the +1 here is necessary because .size() doesn't account for the null terminator that decode_html_entities will want.
	// either way, one extra byte won't hurt

	// this is kind of a rat's nest both to avoid allocations (each regex replace requires one, but decode_html_entities is in-place)
	// and because we have to jump through some hoops in order to read from a string_view, because std::regex doesn't know about string views
	// (we take a string_view to avoid an allocation on the way in, at least)

	std::string line_break_temp_buffer(to_strip.size() + 1, '\0');

	auto end_of_output = std::regex_replace(&line_break_temp_buffer[0], to_strip.begin(), to_strip.end(), replace_line_breaks, "\n");

	std::string par_break_temp_buffer(line_break_temp_buffer.size() + 1, '\0');

	end_of_output = std::regex_replace(&par_break_temp_buffer[0], &line_break_temp_buffer[0], end_of_output, replace_paragraph_breaks, "\n\n");
	par_break_temp_buffer.resize(end_of_output - &par_break_temp_buffer[0]);

	std::string output_buffer = std::regex_replace(par_break_temp_buffer, remove_tags, "");

	const size_t decoded_length = decode_html_entities_utf8(&output_buffer[0], nullptr);

	output_buffer.resize(decoded_length);
	return output_buffer;
}

std::string& bulk_replace_mentions(std::string& str, const std::vector<std::pair<std::string_view, std::string_view>>& to_replace)
{
	size_t match_idx = 0;
	for (const auto& pair : to_replace)
	{
		match_idx = str.find(pair.first, match_idx);

		//  only do this if a match was found AND it has an @ beforehand. 
		if (match_idx != std::string::npos && match_idx != 0 && str[match_idx - 1] == '@')
		{
			str.replace(match_idx, pair.first.size(), pair.second);
			match_idx += pair.second.size();
		}
	}

	return str;
}
