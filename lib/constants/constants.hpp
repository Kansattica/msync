#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <string_view>

// inline should make sure there's only one between all the files
inline constexpr std::string_view Account_Directory{"msync_accounts"};

inline constexpr std::string_view User_Options_Filename{"user.config"};
inline constexpr std::string_view List_Options_Filename{"lists.config"};

inline constexpr std::string_view Fav_Queue_Filename{"fav.queue"};
inline constexpr std::string_view Boost_Queue_Filename{"boost.queue"};
inline constexpr std::string_view Post_Queue_Filename{"post.queue"};

inline constexpr std::string_view File_Queue_Directory{"queuedposts"};

#endif