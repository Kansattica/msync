#ifndef MSYNC_NET_HPP
#define MSYNC_NET_HPP

#include "../netinterface/net_interface.hpp"

net_response simple_post(std::string_view url, std::string_view access_token);
net_response simple_delete(std::string_view url, std::string_view access_token);
net_response new_status(std::string_view url, std::string_view access_token, const status_params& params);
net_response upload_media(std::string_view url, std::string_view access_token, const fs::path& file, const std::string& description);
net_response get_timeline_and_notifs(std::string_view url, std::string_view access_token, const timeline_params& params, unsigned int limit);
#endif
