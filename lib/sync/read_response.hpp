#ifndef _READ_RESPONSE_
#define _READ_RESPONSE_
#include <string>

#include "../entities/entities.hpp"

std::string read_error(const std::string& response_json);

mastodon_status read_status(const std::string& status_json);
std::string read_upload_id(const std::string& attachment_json);

#endif
