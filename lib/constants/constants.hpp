#ifndef _CONSTANTS_HPP_
#define _CONSTANTS_HPP_

#include <string_view>

constexpr std::string_view Account_Directory{ "msync_accounts" };

constexpr std::string_view User_Options_Filename{ "user.config" };
constexpr std::string_view List_Options_Filename{ "lists.config" };

constexpr std::string_view Queue_Filename{ "queued.list" };

constexpr std::string_view File_Queue_Directory{ "queuedposts" };

constexpr std::string_view Home_Timeline_Filename{ "home.list" };
constexpr std::string_view Notifications_Filename{ "notifications.list" };
constexpr std::string_view Direct_Messages_Filename{ "dm.list" };
#endif