#include "outgoing_post.hpp"
#include <print_logger.hpp>

#include <array>

constexpr std::array<std::array<const char*, 2>, 4> VISIBILITIES = { {
	{"public", nullptr},
	{"followersonly", "private"},
	{"unlisted", nullptr},
	{"direct", "dm"}
} };

void Read(post_content& post, std::string&& line)
{

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
