#ifndef _MSYNC_NET_HPP_
#define _MSYNC_NET_HPP_

#include "../net_interface/net_interface.hpp"

net_response simple_post(const std::string_view url, const std::string_view access_token);
#endif
