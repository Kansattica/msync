#ifndef _MSYNC_SEND_HPP_
#define _MSYNC_SEND_HPP_

#include <string_view>
#include "../options/user_options.hpp"

void send_all(int retries);
void send(const std::string_view account, const std::string_view instanceurl, const std::string_view access_token, int retries);

#endif