#ifndef _MSYNC_SEND_HPP_
#define _MSYNC_SEND_HPP_

#include <string>
#include "../options/user_options.hpp"

void send_all(int retries);
void send(const std::string& account, const std::string& instanceurl, const std::string& access_token, int retries);

#endif