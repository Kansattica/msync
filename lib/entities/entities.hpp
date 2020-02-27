#ifndef _MASTODON_ENTITIES_HPP_
#define _MASTODON_ENTITIES_HPP_

#include <optional>
#include <vector>

struct mastodon_account_field
{
	std::string name;
	std::string value;
};

struct mastodon_poll_option
{
	std::string title;
	int votes = 0;
};

struct mastodon_poll
{
	std::string id;
	std::string expires_at;
	bool expired = true;
	int total_votes = 0;
	bool you_voted = false;
	std::vector<int> voted_for;
	std::vector<mastodon_poll_option> options;
};

struct mastodon_account
{
	std::string id;
	std::string account_name; // username if a local account, username@domain.egg if a remote account
	std::string display_name;
	std::string note;
	std::string url;
	std::string avatar;
	std::vector<mastodon_account_field> fields;
	bool is_bot = false;
};

struct mastodon_attachment
{
	std::string url;
	std::string description;
};

struct mastodon_status
{
	std::string id;
	std::string url;
	std::string content_warning;
	std::string content;
	std::string visibility;
	std::string created_at;
	std::string reply_to_post_id;
	std::string original_post_url; // if this is a boost of another post 
	std::string boosted_by; // if this is a boost of another post 
	std::string boosted_by_display_name; // if this is a boost of another post 
	bool boosted_by_bot = false; // if this is a boost of another post 
	unsigned int favorites = 0;
	unsigned int boosts = 0;
	unsigned int replies = 0;
	std::vector<mastodon_attachment> attachments;
	mastodon_account author;
	std::optional<mastodon_poll> poll;
};

enum class notif_type
{
	unknown, follow, mention, boost, favorite, poll
};

struct mastodon_notification
{
	std::string id;
	notif_type type = notif_type::unknown;
	std::string created_at;
	mastodon_account account;
	std::optional<mastodon_status> status;
};

#endif
