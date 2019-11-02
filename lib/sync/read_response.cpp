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
	toreturn.content_warning = clean_up_html(parsed["spoiler_text"].get<std::string>());
	toreturn.content = clean_up_html(parsed["content"].get<std::string>());
	toreturn.visibility = parsed["visibility"].get<std::string>();

	return toreturn;
}

std::string read_upload_id(const std::string& attachment_json)
{
	return json::parse(attachment_json)["id"].get<std::string>();

}
