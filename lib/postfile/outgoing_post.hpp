#ifndef OUTGOING_POST_HPP
#define OUTGOING_POST_HPP

#include <vector>
#include <string>
#include <string_view>

#include "../filebacked/file_backed.hpp"

enum class visibility
{
	default_vis, pub, priv, unlisted, direct
};

enum class raw_text_mode
{
	unset, raw, cooked
};

struct post_content
{
	std::string text;
	std::string reply_id;
	std::string reply_to_id;
	std::string content_warning;
	std::vector<std::string> attachments;
	std::vector<std::string> descriptions;
	visibility vis = visibility::default_vis;

	std::string visibility_string() const;

	raw_text_mode is_raw = raw_text_mode::unset;
};

bool Read(post_content&, std::string&&);
void Write(post_content&&, std::ofstream&);

using outgoing_post = file_backed<post_content, Read, Write, false, false, false>;
using readonly_outgoing_post = file_backed<post_content, Read, Write, false, false, true>;
#endif
