#ifndef _USER_OPTIONS_HPP_
#define _USER_OPTIONS_HPP_

#include <string>

#include "filesystem.hpp"

#include "option_file.hpp"

enum class list_operations { add, remove, clear };

enum class sync_settings { off, newest_first, oldest_first };

enum class user_option {
  accountname,
  instanceurl,
  accesstoken,
  username,
  password,
  clientsecret,
  home,
  dms,
  notifications,
};

struct user_options {
public:
  user_options(fs::path toread) : backing(toread){};

  const std::string* const get_option(user_option toget) const;
  void set_option(user_option toset, std::string&& value);

private:
  option_file backing;
};
#endif
