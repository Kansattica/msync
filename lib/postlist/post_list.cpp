#include "post_list.hpp"

void print(std::ofstream& out, const char* key, const std::string& val)
{
	if (!val.empty())
		out << key << val << '\n';
}

std::ofstream& operator<<(std::ofstream& out, const mastodon_status& status)
{
	print(out, "id: ", status.id);
	print(out, "url: ", status.url);
	print(out, "author: ", status.author.account_name);
	print(out, "reply to: ", status.reply_to_post_id);
	print(out, "boost of: ", status.original_post_url);
	print(out, "cw: ", status.content_warning);
	print(out, "body: ", status.content);
	print(out, "visibility: ", status.visibility);
	print(out, "posted on: ", status.created_at);

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

	out << "favs: " << status.favorites << "boosts: " << status.boosts << "replies: " << status.replies << '\n';
}

const char* notification_verb(notif_type t)
{
	switch (t)
	{
	case notif_type::boost:
		return " boosted your post:";
	case notif_type::favorite:
		return " favorited your post:";
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
	out << notification.account.account_name << notification_verb(notification.type) << '\n';
	
	if (notification.status.has_value())
	{
		out << notification.status.value();
	}
}