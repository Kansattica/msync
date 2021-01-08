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

std::string read_error(const std::string_view response_json)
{
	const auto parsed = json::parse(response_json, nullptr, false); //don't throw on a bad parse

	// a parse fail is fine here because it might just be a plain error string
	// from nginx, for instance
	if (parsed == json::value_t::discarded)
	{
		return {};
	}

	return get_if_set<std::string>(parsed, "error"sv);
}

void from_json(const json& j, mastodon_poll_option& option)
{
	j["title"].get_to(option.title);
	j["votes_count"].get_to(option.votes);
}

void from_json(const json& j, mastodon_poll& poll)
{
	j["id"].get_to(poll.id);
	
	const auto& expiry = j["expires_at"];
	if (!expiry.is_null())
		expiry.get_to(poll.expires_at);
	j["expired"].get_to(poll.expired);
	j["votes_count"].get_to(poll.total_votes);
	j["options"].get_to(poll.options);

	const auto voted = j.find("voted"sv);
	if (voted != j.end() && voted->is_boolean())
	{
		voted->get_to(poll.you_voted);
	}

	// this ones can be missing on your own polls
	const auto own_votes = j.find("own_votes"sv);
	if (own_votes != j.end() && own_votes->is_array())
	{
		own_votes->get_to(poll.voted_for);
	}
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
	account.fields = get_if_set<std::vector<mastodon_account_field>>(j, "fields"sv);
	j["bot"].get_to(account.is_bot);
}

void from_json(const json& j, mastodon_attachment& attachment)
{
	j["url"].get_to(attachment.url);
	attachment.description = get_if_set<std::string>(j, "description"sv);
}

std::vector<std::pair<std::string_view, std::string_view>> get_mentions(const json& j)
{
	std::vector<std::pair<std::string_view, std::string_view>> to_return;

	const auto& mentions = j["mentions"];

	if (mentions.is_null() || !mentions.is_array()) { return to_return; }

	for (const auto& mention : mentions)
	{
		to_return.emplace_back(mention["username"].get<std::string_view>(), mention["acct"].get<std::string_view>());
	}

	return to_return;
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

	// basically, if this post is a reblog, we want to get the rest of the stuff out of the nested reblog object.

	const auto is_reblog = j.find("reblog"sv);
	const json* post = &j;
	if (is_reblog != j.end() && is_reblog->is_object())
	{
		post = &*is_reblog;
		j["account"]["acct"].get_to(status.boosted_by);
		j["account"]["bot"].get_to(status.boosted_by_bot);
		j["account"]["display_name"].get_to(status.boosted_by_display_name);
		post->at("uri").get_to(status.original_post_url);
	}

	bulk_replace_mentions(status.content, get_mentions(*post));
	status.reply_to_post_id = get_if_set<std::string>(*post, "in_reply_to_id"sv);

	post->at("created_at").get_to(status.created_at);
	post->at("favourites_count").get_to(status.favorites);
	post->at("reblogs_count").get_to(status.boosts);
	post->at("replies_count").get_to(status.replies);

	post->at("media_attachments").get_to(status.attachments);
	post->at("account").get_to(status.author);

	const auto poll = j.find("poll"sv);
	if (poll != j.end() && poll->is_object())
	{
		status.poll = poll->get<mastodon_poll>();
	}
}

void from_json(const json& j, mastodon_context& context)
{
	j["ancestors"].get_to(context.ancestors);
	j["descendants"].get_to(context.descendants);
}

NLOHMANN_JSON_SERIALIZE_ENUM(notif_type, {
		{ notif_type::unknown, "???" }, //nlohmann json will pick the first one in the list if it can't parse
		{ notif_type::follow, "follow" },
		{ notif_type::mention, "mention" },
		{ notif_type::boost, "reblog" },
		{ notif_type::poll, "poll" },
		{ notif_type::favorite, "favourite" },
	})

void from_json(const json& j, mastodon_notification& notif)
{
	j["id"].get_to(notif.id);
	j["type"].get_to(notif.type);
	j["created_at"].get_to(notif.created_at);
	j["account"].get_to(notif.account);
	const auto status = j.find("status"sv);
	if (status != j.end() && status->is_object())
	{
		notif.status = status->get<mastodon_status>();
	}
}

mastodon_status read_status(const std::string_view status_json)
{
	return json::parse(status_json).get<mastodon_status>();
}

std::vector<mastodon_status> read_statuses(const std::string_view timeline_json)
{
	return json::parse(timeline_json).get<std::vector<mastodon_status>>();
}

mastodon_notification read_notification(const std::string_view notification_json)
{
	return json::parse(notification_json).get<mastodon_notification>();
}

std::vector<mastodon_notification> read_notifications(const std::string_view notifications_json)
{
	return json::parse(notifications_json).get<std::vector<mastodon_notification>>();
}

mastodon_context read_context(const std::string_view context_json)
{
	return json::parse(context_json).get<mastodon_context>();
}

std::string read_upload_id(const std::string_view attachment_json)
{
	return json::parse(attachment_json)["id"].get<std::string>();
}
