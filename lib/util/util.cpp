#include "util.hpp"

#include <regex>
#include <memory>


std::string make_api_url(const std::string_view instance_url, const std::string_view api_route)
{
    std::string to_return{"https://"};
    to_return.reserve(instance_url.size() + api_route.size() + to_return.size());
    to_return.append(instance_url).append(api_route);
    return to_return;
}

std::optional<parsed_account> parse_account_name(const std::string& name)
{
    const static std::regex account_name{R"(@?([_a-z0-9]+)@(?:https?://)?([a-z0-9-]+\.[a-z0-9-]+(?:\.[a-z0-9-]+)?)[, =/\\?]*$)", std::regex::ECMAScript | std::regex::icase};

    std::smatch results;
    if (std::regex_match(name, results, account_name))
    {
        return parsed_account{results[1], results[2]};
    }

    return {};
}

// if src is null, modifies dest in place
extern "C" size_t decode_html_entities_utf8(char *dest, const char *src);

std::string clean_up_html(const std::string& to_strip)
{
	const static std::regex remove_tags{ "<[^<]*>" };

	// make_unique will zero out the char buffer
	// so no need to worry about null terminators
	// see: https://stackoverflow.com/questions/42140212/does-make-unique-value-initializes-char-array
	
	// regex_replace will always keep the string the same length or make it shorter, so it won't cause an overflow
	// the author of decode_html_entities says the same thing
	// I think the +1 here is necessary because .size() doesn't account for the null terminator
	// either way, one extra byte won't hurt
	auto output_buffer = std::make_unique<char[]>(to_strip.size() + 1);

	const auto end_of_output = std::regex_replace(output_buffer.get(), to_strip.begin(), to_strip.end(), remove_tags, "");

	size_t decoded_length = decode_html_entities_utf8(output_buffer.get(), nullptr);

	return std::string(output_buffer.get(), decoded_length);
}
