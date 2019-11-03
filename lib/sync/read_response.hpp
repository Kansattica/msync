#ifndef _READ_RESPONSE_
#define _READ_RESPONSE_
#include <string>

struct mastodon_status
{
	std::string id;
	std::string url;
	std::string content_warning;
	std::string content;
	std::string visibility;
};

mastodon_status read_status(const std::string& status_json);
std::string read_upload_id(const std::string& attachment_json);

#endif