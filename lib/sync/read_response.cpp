#include "read_response.hpp"

#include <nlohmann/json.hpp>
#include <print_logger.hpp>

#include <string_view>

#include "../util/util.hpp"

using json = nlohmann::json;

template <typename String>
String get_if_set(const json& parsed, const char* key)
{
	const auto val = parsed.find(key);
	if (val == parsed.end())
		return "";
	return val->get<String>();
}

mastodon_status read_status(const std::string& status_json)
{
	const auto parsed = json::parse(status_json);

	mastodon_status toreturn;

	toreturn.id = parsed["id"].get<std::string>();
	toreturn.url = parsed["uri"].get<std::string>();

	// the mastodon API says these will always be here, but do this to be safe.
	// it also says that spoiler_text won't have html, but I'm not sure how correct that is
	// i suspect it might at least have HTML entities that have to be cleaned up
	toreturn.content_warning = clean_up_html(get_if_set<std::string_view>(parsed, "spoiler_text"));
	toreturn.content = clean_up_html(get_if_set<std::string_view>(parsed, "content"));

	toreturn.visibility = parsed["visibility"].get<std::string>();

	return toreturn;
}

std::string read_upload_id(const std::string& attachment_json)
{
	return json::parse(attachment_json)["id"].get<std::string>();

}
