#ifndef _CONSTANTS_HPP_
#define _CONSTANTS_HPP_

// boost doesn't know what to do with std::string_views and paths.
#if MSYNC_USE_BOOST
#include <string>
#define CONSTANT_PATH_TYPE std::string
#define CONSTANT_PATH_DECLARATION const CONSTANT_PATH_TYPE
#define CONSTEXPR_IF_NOT_BOOST 
#else
#include <string_view>
#define CONSTANT_PATH_TYPE std::string_view
#define CONSTANT_PATH_DECLARATION constexpr CONSTANT_PATH_TYPE
#define CONSTEXPR_IF_NOT_BOOST constexpr
#endif

inline CONSTANT_PATH_DECLARATION Account_Directory{ "msync_accounts" };

inline CONSTANT_PATH_DECLARATION User_Options_Filename{ "user.config" };
inline CONSTANT_PATH_DECLARATION List_Options_Filename{ "lists.config" };

inline CONSTANT_PATH_DECLARATION Queue_Filename{ "queued.list" };

inline CONSTANT_PATH_DECLARATION File_Queue_Directory{ "queuedposts" };

inline CONSTANT_PATH_DECLARATION Home_Timeline_Filename{ "home.list" };
inline CONSTANT_PATH_DECLARATION Notifications_Filename{ "notifications.list" };
inline CONSTANT_PATH_DECLARATION Direct_Messages_Filename{ "dm.list" };

#endif