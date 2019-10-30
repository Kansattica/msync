#ifndef _OUTGOING_POST_HPP_
#define _OUTGOING_POST_HPP_

#include <vector>
#include <string>
#include <string_view>

#include "../filebacked/file_backed.hpp"

enum class visibility
{
	pub, priv, unlisted, direct
};

enum class raw_text_mode
{
	unset, raw, cooked
};

struct post_content
{
	std::string text;
	std::string reply_to_id;
	std::string content_warning;
	std::vector<std::string> attachments;
	std::vector<std::string> descriptions;
	visibility vis = visibility::pub;

	std::string visibility_string() const;

	raw_text_mode is_raw = raw_text_mode::unset;
};

void Read(post_content&, std::string&&);
void Write(post_content&&, std::ofstream&);

using outgoing_post = file_backed<post_content, Read, Write, false, false>;
#endif
