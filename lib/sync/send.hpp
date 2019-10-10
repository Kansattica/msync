#ifndef _MSYNC_SEND_HPP_
#define _MSYNC_SEND_HPP_

#include <string_view>
#include "../options/user_options.hpp"

#include "../net_interface/net_interface.hpp"


template <post_request post>
void send_all(int retries);

template <post_request post>
void send(const std::string& account, const std::string& instanceurl, const std::string& access_token, int retries);

#endif