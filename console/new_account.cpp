#include "new_account.hpp"

#include "../lib/options/global_options.hpp"
#include "../lib/util/util.hpp"
#include "../lib/options/option_enums.hpp"    // for user_option, user_optio...
#include "../lib/options/option_file.hpp"     // for string
#include "../lib/options/user_options.hpp"    // for user_options

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print_logger.hpp>

#include <string>
#include <optional>                           // for optional
#include <utility>                            // for pair

using json = nlohmann::json;

constexpr auto scopes = "write:favourites write:media write:statuses read:notifications read:statuses";
constexpr auto urlscopes = "write:favourites%20write:media%20write:statuses%20read:notifications%20read:statuses";
constexpr auto redirect_uri = "urn:ietf:wg:oauth:2.0:oob";

void make_new_account(const std::string& accountname)
{
    auto useraccountpair = options().select_account(accountname);

    // see: https://docs.joinmastodon.org/api/authentication/

    // if no user was found, make a new one
    if (useraccountpair == nullptr)
    {
        pl() << "Creating new account for " << accountname << "\n";
        const auto parsed = parse_account_name(accountname);
        if (!parsed.has_value())
        {
            pl() << "Could not parse a username and instance name from: " << accountname << ". It should look like: username@instance.url\n";
            return;
        }
        // make a new account
        useraccountpair = &options().add_new_account(accountname);
        useraccountpair->second.set_option(user_option::account_name, parsed->username);
        useraccountpair->second.set_option(user_option::instance_url, parsed->instance);
        useraccountpair->second.set_option(user_option::file_version, "1");
    }
    else
    {
        pl() << "Existing user found.\n";
    }
    
	auto& useraccount = useraccountpair->second;

    // register the application with mastodon if needed
    auto client_id = useraccount.try_get_option(user_option::client_id);
    const auto client_secret = useraccount.try_get_option(user_option::client_secret);

    const auto& instanceurl = useraccount.get_option(user_option::instance_url);
    if (client_id == nullptr || client_secret == nullptr)
    {
        pl() << "Registering app with " << instanceurl << '\n';
        const auto r = cpr::Post(cpr::Url{make_api_url(instanceurl, "/api/v1/apps")},
                           cpr::Parameters{{"client_name", "msync"}, {"redirect_uris", redirect_uri}, {"scopes", scopes}, {"website", "https://github.com/kansattica/msync"}});

        if (r.error)
        {
            pl() << "Could not register app with server. Responded with error code " << r.status_code << ": " << r.error.message << '\n';
            pl() << "Please try again.\n";
            return;
        }

        const json parsed = json::parse(r.text);
        useraccount.set_option(user_option::client_id, parsed["client_id"].get<std::string>());
        useraccount.set_option(user_option::client_secret, parsed["client_secret"].get<std::string>());
        pl() << "Registered!\n";
    }
    else
    {
        pl() << "App already registered.\n";
    }

    if (client_id == nullptr)
        client_id = useraccount.try_get_option(user_option::client_id);

    const auto authcode = useraccount.try_get_option(user_option::auth_code);
    if (authcode == nullptr)
    {
        const auto& foundaccountname = useraccount.get_option(user_option::account_name);
        pl() << "Please open this URL in your browser:\n"
           << "https://" << instanceurl << "/oauth/authorize?response_type=code&client_id=" << *client_id
           << "&redirect_uri=" << redirect_uri << "&scope=" << urlscopes << '\n'
           << "Enter your authorization code like so:\n"
           << "msync config authcode <the authorization code from the site> -a " << foundaccountname << "@" << instanceurl << '\n'
           << "then run this again:\n"
           << "msync new -a " << foundaccountname << "@" << instanceurl << '\n'
           << "You can shorten the username part, as long as msync can figure out which registered account you're talking about.\n"
           << "If this is your only account, you can leave the -a part off altogether.\n";
        return;
    }

    const auto access_token = useraccount.try_get_option(user_option::access_token);
    if (access_token != nullptr)
    {
        pl() << "Your account is already registered! You're done!\n";
        return;
    }

    pl() << "Getting access token from server with authorization code.\n";
    const auto response = cpr::Post(cpr::Url{make_api_url(instanceurl, "/oauth/token")}, 
        cpr::Parameters{{"client_id", *client_id}, {"client_secret", *client_secret}, {"grant_type", "authorization_code"}, 
        {"code", *authcode}, {"redirect_uri", redirect_uri}});

    useraccount.set_option(user_option::auth_code, "");
    if (response.error)
    {
        const auto foundaccountname = *useraccount.try_get_option(user_option::account_name);
        pl() << "Could not get access token from server. Authorization codes can only be used once, so it's been deleted and you should get another one.\n"
           << "Please open this URL in your browser:\n"
           << "https://" << instanceurl << "/oauth/authorize?response_type=code&client_id=" << *client_id
           << "&redirect_uri=" << redirect_uri << "&scope=" << urlscopes << '\n'
           << "Enter your authorization code like so:\n"
           << "msync config authcode <the authorization code from the site> -a " << foundaccountname << "@" << instanceurl << '\n'
           << "then run this again:\n"
           << "msync new -a " << foundaccountname << "@" << instanceurl << '\n';
        return;
    }

    const json parsed = json::parse(response.text);
    useraccount.set_option(user_option::access_token, parsed["access_token"].get<std::string>());
    pl() << "Done! You're ready to start using this account."; //main will add its own newline before it exits.

    //don't need these any more once we're good
    useraccount.set_option(user_option::client_id, "");
    useraccount.set_option(user_option::client_secret, "");
}
