#ifndef _QUEUE_LIST_HPP_
#define _QUEUE_LIST_HPP_

#include <vector>
#include <string>

#include "../filebacked/file_backed.hpp"

enum class visibility
{
	pub, priv, unlisted, direct
};

struct post_content
{
	std::string text;
	std::string reply_to_id;
	std::string content_warning;
	std::vector<std::string> attachments;
	visibility vis = visibility::pub;
	bool seen_line = false;
};

void Read(post_content&, std::string&&);
void Write(post_content&&, std::ofstream&);

using outgoing_post = file_backed<post_content, Read, Write>;
#endif
