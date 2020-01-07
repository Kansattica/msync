#ifndef _READ_RESPONSE_
#define _READ_RESPONSE_
#include <string>
#include <string_view>
#include <vector>

#include "../entities/entities.hpp"

std::string read_error(std::string_view response_json);

mastodon_status read_status(std::string_view status_json);
std::vector<mastodon_status> read_statuses(std::string_view timeline_json);
mastodon_notification read_notification(std::string_view notification_json);
std::vector<mastodon_notification> read_notifications(std::string_view notifications_json);
std::vector<mastodon_dm> read_dms(std::string_view dm_json);
std::string read_upload_id(std::string_view attachment_json);

#endif
