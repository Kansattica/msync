#include "newaccount.hpp"
#include "../lib/options/options.hpp"
#include <print_logger.hpp>

void make_new_account(const std::string& accountname)
{
    print_logger<logtype::normal> pl;
    auto useraccount = options.select_account(accountname);

    // see: https://docs.joinmastodon.org/api/authentication/

    // if no user was found, make a new one
    if (useraccount == nullptr)
    {
        auto parsed = parse_account_name(accountname);
        if (!parsed.has_value())
        {
            pl << "Could not parse a username and instance name from: " << accountname << ". Please ensure it is of the form username@instance.url\n";
            return;
        }
        // make a new account
        user_options& newuser = options.add_new_account(accountname);
        newuser.set_option(user_option::accountname, parsed->username);
        newuser.set_option(user_option::instanceurl, parsed->instance);
        // register the application with mastodon
    }
}