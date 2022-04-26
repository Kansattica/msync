#include "post_list.hpp"

#include <algorithm>

void print(std::ostream& out, const char* key, const std::string& val, bool newline = true)
{
	if (!val.empty())
	{
		out << key << val;
		if (newline)
			out << '\n';
	}
}

void print_author(std::ostream& out, const char* key, const std::string& display_name, const std::string& account, bool bot, bool newline = true)
{ 
	// this is so that posts that aren't boosts don't print that boosted_by section
	if (account.empty()) { return; }

	out << key << display_name << " (@" << account << ')';

	if (bot) out << " [bot]";

	if (newline) out << '\n';
}

void print(std::ostream& out, const char* key, bool val, bool newline = true)
{
	if (val)
	{
		out << key;
		if (newline)
			out << '\n';
	}
}

std::ostream& operator<<(std::ostream& out, const mastodon_poll& poll)
{
	print(out, "poll id: ", poll.id);
	print(out, poll.expired ? "expired at: " : "expires at: ", poll.expires_at);
	for (size_t i = 0; i < poll.options.size(); ++i)
	{
		out << " - " << poll.options[i].title << ' ' << 
			poll.options[i].votes << '/' << poll.total_votes << " votes (" << (poll.total_votes == 0 ? 0 : ((double)poll.options[i].votes / poll.total_votes) * 100) << "%)";
		if (std::find(poll.voted_for.begin(), poll.voted_for.end(), i) != poll.voted_for.end())
		{
			out << " [your vote]";
		}
		out << '\n';
	}

	return out;
}

std::ostream& operator<<(std::ostream& out, const mastodon_status& status)
{
	print(out, "status id: ", status.id);
	print(out, "url: ", status.url);
	print_author(out, "author: ", status.author.display_name, status.author.account_name, status.author.is_bot);
	print_author(out, "boosted by: ", status.boosted_by_display_name, status.boosted_by, status.boosted_by_bot);
	print(out, "reply to: ", status.reply_to_post_id);
	print(out, "boost of: ", status.original_post_url);
	print(out, "cw: ", status.content_warning);
	print(out, "body: ", status.content);

	for (const auto& attachment : status.attachments)
	{
		out << "attached: " << attachment.url << '\n';
		if (!attachment.description.empty())
		{
			out << "description: " << attachment.description << '\n';
		}
	}

	if (status.poll.has_value())
	{
		out << *status.poll;
	}

	print(out, "visibility: ", status.visibility);
	print(out, "posted on: ", status.created_at);
	out << status.favorites << " favs | " << status.boosts << " boosts | " << status.replies << " replies";

	return out;
}

const char* notification_verb(notif_type t)
{
	switch (t)
	{
	case notif_type::favorite:
		return " favorited your post:";
	case notif_type::boost:
		return " boosted your post:";
	case notif_type::mention:
		return " mentioned you:";
	case notif_type::follow:
		return " followed you.";
	case notif_type::poll:
		return "'s poll ended:";
	case notif_type::update:
		return " updated their post:";
	default:
		return " ??? ";
	}
}

std::ostream& operator<<(std::ostream& out, const mastodon_notification& notification)
{
	out << "notification id: " << notification.id << '\n';
	out << "at " << notification.created_at << ", ";
	print_author(out, "", notification.account.display_name, notification.account.account_name, notification.account.is_bot, false);
	out << notification_verb(notification.type);

	if (notification.status.has_value())
	{
		out << '\n';
		out << *notification.status;
	}

	return out;
}
