#include "read_response.hpp"

#include <nlohmann/json.hpp>
#include <print_logger.hpp>

#include "../util/util.hpp"

using json = nlohmann::json;

mastodon_status read_status(const std::string& status_json)
{
	const auto parsed = json::parse(status_json);

	mastodon_status toreturn;

	toreturn.id = parsed["id"].get<std::string>();
	toreturn.url = parsed["uri"].get<std::string>();
	toreturn.content_warning = parsed["spoiler_text"].get<std::string>();
	toreturn.content = strip_html_tags(parsed["content"].get<std::string>());
	toreturn.visibility = parsed["visibility"].get<std::string>();

	return toreturn;
}
