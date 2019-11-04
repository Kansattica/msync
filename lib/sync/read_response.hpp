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

std::string read_error(const std::string& response_json);

mastodon_status read_status(const std::string& status_json);
std::string read_upload_id(const std::string& attachment_json);

#endif
