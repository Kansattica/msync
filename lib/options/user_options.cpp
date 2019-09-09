#include "user_options.hpp"

#include <array>

const auto optionsToKeys =
    std::array<const std::string,
               static_cast<int>(user_option::notifications) + 1>(
        {"account_name", "instance_url", "access_token", "username", "password",
         "client_secret", "pull_home", "pull_dms", "pull_notifications"});

const std::string* const user_options::get_option(user_option opt) const {
  const auto val =
      backing.parsed_options.find(optionsToKeys[static_cast<int>(opt)]);
  if (val == backing.parsed_options.end())
    return nullptr;

  return &val->second;
}

void user_options::set_option(user_option opt, std::string&& value) {
  backing.parsed_options.insert_or_assign(optionsToKeys[static_cast<int>(opt)],
                                          std::forward<std::string>(value));
}