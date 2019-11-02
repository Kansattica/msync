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

extern "C" size_t decode_html_entities_utf8(char *dest, const char *src);

std::string clean_up_html(const std::string& to_strip)
{
	const static std::regex remove_tags{ "<[^<]*>" };

	auto tags_stripped = std::regex_replace(to_strip, remove_tags, "");
	
	auto output_buffer = std::make_unique<char[]>(tags_stripped.size() + 1);

	size_t decoded_length = decode_html_entities_utf8(output_buffer.get(), tags_stripped.c_str());

	return std::string(output_buffer.get(), decoded_length);
}
