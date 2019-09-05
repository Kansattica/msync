#include <string>
#include <optional>

#include "filesystem.hpp"

#include "option_file.hpp"

enum class sync_settings
{
    off,
    newest_first,
    oldest_first
};

enum class user_option
{
    accountname,
    instanceurl,
    accesstoken,
    username,
    password,
    clientsecret,
    home,
    dms,
    notifications,

    // these guys have to be handled specially
    // fix it later because having special enums is gonna cause bugs later.
    addlist,
    removelist,
    newaccount,
    show
};

struct user_options
{
public:
    user_options(fs::path toread) : backing(toread){};

    std::optional<std::string> get_option(user_option toget);
    void set_option(user_option toset, std::string value);

private:
    option_file backing;
};
