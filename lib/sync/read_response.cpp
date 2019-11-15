#include "read_response.hpp"

#include <nlohmann/json.hpp>
#include <print_logger.hpp>

#include "../util/util.hpp"

using json = nlohmann::json;

using namespace std::string_view_literals;

template <typename T>
T get_if_set(const json& parsed, const std::string_view key)
{
	const auto val = parsed.find(key);
	if (val == parsed.end() || val->is_null())
		return {};
	return val->get<T>();
}

std::string get_reblog_uri_if_set(const json& parsed)
{
	const auto val = parsed.find("reblog"sv);
	if (val == parsed.end() || val->is_null())
		return {};
	return (*val)["uri"].get<std::string>();
}

std::string read_error(const std::string_view response_json)
{
	const auto parsed = json::parse(response_json, nullptr, false); //don't throw on a bad parse

	// a parse fail is fine here because it might just be a plain error string
	// from nginx, for instance
	if (parsed == json::value_t::discarded)
	{
		return "";
	}

	return get_if_set<std::string>(parsed, "error"sv);
}

void from_json(const json& j, mastodon_account_field& field)
{
	j["name"].get_to(field.name);
	field.value = clean_up_html(j["value"].get<std::string_view>()); // these can be HTML if they're links
}

void from_json(const json& j, mastodon_account& account)
{
	j["id"].get_to(account.id);
	j["acct"].get_to(account.account_name);
	j["display_name"].get_to(account.display_name);
	account.note = clean_up_html(j["note"].get<std::string_view>());
	j["url"].get_to(account.url);
	j["avatar"].get_to(account.avatar);
	account.fields = get_if_set<std::vector<mastodon_account_field>>(j, "fields");
	j["bot"].get_to(account.is_bot);
}

void from_json(const json& j, mastodon_attachment& attachment)
{
	j["url"].get_to(attachment.url);
	attachment.description = get_if_set<std::string>(j, "description"sv);
}

void from_json(const json& j, mastodon_status& status)
{
	j["id"].get_to(status.id);
	j["uri"].get_to(status.url);

	// the mastodon API says these will always be here, but do this to be safe.
	// it also says that spoiler_text won't have html, but I'm not sure how correct that is
	// i suspect it might at least have HTML entities that have to be cleaned up
	status.content_warning = clean_up_html(get_if_set<std::string_view>(j, "spoiler_text"sv));
	status.content = clean_up_html(get_if_set<std::string_view>(j, "content"sv));

	j["visibility"].get_to(status.visibility);
	j["created_at"].get_to(status.created_at);
	
	status.reply_to_post_id = get_if_set<std::string>(j, "in_reply_to_id"sv);
	status.original_post_url = get_reblog_uri_if_set(j);
	j["favourites_count"].get_to(status.favorites);
	j["reblogs_count"].get_to(status.boosts);
	j["replies_count"].get_to(status.replies);

	j["media_attachments"].get_to(status.attachments);
	j["account"].get_to(status.author);
}

mastodon_status read_status(const std::string_view status_json)
{
	return json::parse(status_json).get<mastodon_status>();
}

std::string read_upload_id(const std::string_view attachment_json)
{
	return json::parse(attachment_json)["id"].get<std::string>();
}
