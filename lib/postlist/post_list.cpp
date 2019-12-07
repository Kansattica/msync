#include "post_list.hpp"

void print(std::ofstream& out, const char* key, const std::string& val, bool newline = true)
{
	if (!val.empty())
	{
		out << key << val;
		if (newline)
			out << '\n';
	}
}

void print(std::ofstream& out, const char* key, bool val, bool newline = true)
{
	if (val)
	{
		out << key;
		if (newline)
			out << '\n';
	}
}

std::ofstream& operator<<(std::ofstream& out, const mastodon_status& status)
{
	print(out, "id: ", status.id);
	print(out, "url: ", status.url);
	print(out, "author: ", status.author.account_name, !status.author.is_bot);
	print(out, " [bot]", status.author.is_bot);
	print(out, "boosted by: ", status.boosted_by, !status.boosted_by_bot);
	print(out, " [bot]", status.boosted_by_bot);
	print(out, "reply to: ", status.reply_to_post_id);
	print(out, "boost of: ", status.original_post_url);
	print(out, "cw: ", status.content_warning);
	print(out, "body: ", status.content);
	print(out, "visibility: ", status.visibility);

	if (!status.attachments.empty())
	{
		out << "attachments:\n";
		for (const auto& attachment : status.attachments)
		{
			out << attachment.url << '\n';
			if (!attachment.description.empty())
			{
				out << attachment.description << '\n';
			}
		}
	}

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
	default:
		return " ??? ";
	}
}

std::ofstream& operator<<(std::ofstream& out, const mastodon_notification& notification)
{
	out << "notification id: " << notification.id << '\n';
	out << "at " << notification.created_at << ", ";
	out << notification.account.account_name << (notification.account.is_bot ? " [bot]" : "") << notification_verb(notification.type);
	
	if (notification.status.has_value())
	{
		out << '\n';
		out << notification.status.value();
	}

	return out;
}
