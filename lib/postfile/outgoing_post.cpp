#include "outgoing_post.hpp"
#include <print_logger.hpp>

#include "../util/util.hpp"

#include <array>
#include <string_view>
#include <algorithm>

constexpr std::array<std::array<std::string_view, 2>, 4> VISIBILITIES = { {
	{"public", ""},
	{"followersonly", "private"},
	{"unlisted", ""},
	{"direct", "dm"}
} };

constexpr std::array<std::string_view, 4> OPTIONS = {
	"visibility", "cw", "reply_to", "attach"
};

size_t is_option(const std::string_view line, size_t equals_sign);
bool is_snip(const std::string_view line);
void parse_option(post_content& post, size_t option_index, const std::string_view value);

void Read(post_content& post, std::string&& line)
{
	// there's two kinds of these post files.
	// one has some options on the top, one is just text
	// if the first line is one of the options, then read those until we get to that snip line
	// otherwise, assume it's all just text

	if (post.is_raw == raw_text_mode::raw)
	{
		post.text.reserve(post.text.length() + line.length() + 1);
		if (!post.text.empty())
			post.text.append(1, '\n');
		post.text.append(line);
		return;
	}

	// lines can be one of three types:
	// - have an option
	// - be a "snip" that indicates that the rest of the file is raw
	// - be raw text

	const auto equals = line.find('=');

	size_t option_index = -1;

	if (equals != std::string::npos)
	{
		option_index = is_option(line, equals);
	}

	// if it's an option, parse it
	if (option_index != -1)
	{
		post.is_raw = raw_text_mode::cooked;
		std::string_view option_val{ line };
		option_val.remove_prefix(equals + 1);
		parse_option(post, option_index, option_val);
		return;
	}

	// if it's a snip, everything else is raw
	if (is_snip(line))
	{
		post.is_raw = raw_text_mode::raw;
		return;
	}

	// if it's anything else, must be raw, including this line
	post.is_raw = raw_text_mode::raw;
	Read(post, std::move(line));
	return;
}

void Write(post_content&& post, std::ofstream& of)
{
	if (!post.content_warning.empty())
	{
		of << "cw=" << post.content_warning << '\n';
	}

	if (!post.reply_to_id.empty())
	{
		of << "reply_to=" << post.reply_to_id << '\n';
	}

	if (!post.attachments.empty())
	{
		of << "attach=";
		for (auto& attach : post.attachments)
		{
			of << attach << ',';
		}
		of << '\n';
	}

	of << "visibility=" << VISIBILITIES[(int)post.vis][0] << '\n';

	of << "--- post body below this line ---\n";

	of << post.text;
}

size_t is_option(const std::string_view line, size_t equals_sign)
{
	for (size_t i = 0; i < OPTIONS.size(); i++)
	{
		if (OPTIONS[i].size() != equals_sign)
		{
			// if the option is the wrong length, can't be it
			continue;
		}

		if (std::equal(line.begin(), line.begin() + equals_sign, OPTIONS[i].begin(), OPTIONS[i].end()))
		{
			return i;
		}
	}

	return -1;
}

bool is_snip(const std::string_view line)
{
	if (line.size() < 3)
	{
		return false;
	}

	for (size_t i = 0; i < 3; i++)
	{
		if (line[i] != '-')
		{
			return false;
		}
	}

	return true;
}

visibility parse_visibility(const std::string_view value)
{
	static_assert(VISIBILITIES.size() == 4);
	static_assert(VISIBILITIES[static_cast<int>(visibility::pub)][0] == "public");
	static_assert(VISIBILITIES[static_cast<int>(visibility::priv)][0] == "followersonly");
	static_assert(VISIBILITIES[static_cast<int>(visibility::unlisted)][0] == "unlisted");
	static_assert(VISIBILITIES[static_cast<int>(visibility::direct)][0] == "direct");

	for (size_t visibility_index = 0; visibility_index < VISIBILITIES.size(); visibility_index++)
	{
		for (size_t i = 0; i < VISIBILITIES[visibility_index].size(); i++)
		{
			if (VISIBILITIES[visibility_index][i] == value)
			{
				return static_cast<visibility>(visibility_index);
			}
		}
	}

	pl() << "Bad visibility option: " << value << ". Defaulting to public. Valid options are:\n";
	for (const auto& vis : VISIBILITIES)
	{
		pl() << vis[0];
		if (!vis[1].empty())
		{
			pl() << " or " << vis[1];
		}
		pl() << '\n';
	}
	return visibility::pub;
}

void store_string(std::string& store_in, const std::string_view value)
{
	store_in = std::string{ value };
}

void store_vector(std::vector<std::string>& store_in, const std::string_view value)
{
	auto split = split_string(value, ',');
	store_in.reserve(split.size());
	for (auto& attachment : split)
	{
		store_in.emplace_back(attachment);
	}
}

void parse_option(post_content& post, size_t option_index, const std::string_view value)
{
	static_assert(OPTIONS.size() == 4);
	static_assert(OPTIONS[0] == "visibility");
	static_assert(OPTIONS[1] == "cw");
	static_assert(OPTIONS[2] == "reply_to");
	static_assert(OPTIONS[3] == "attach");

	switch (option_index)
	{
	case 0:
		post.vis = parse_visibility(value);
		break;
	case 1:
		// cw and reply_to just get stored
		store_string(post.content_warning, value);
		break;
	case 2:
		store_string(post.reply_to_id, value);
		break;
	case 3:
		// attachments are a comma-separated list
		store_vector(post.attachments, value);
		break;
	}

}
