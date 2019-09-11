#include "newaccount.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/util/util.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print_logger.hpp>
#include <string>

using json = nlohmann::json;

const auto scopes = "write:favourites write:notifications read:statuses read:lists";
const auto urlscopes = "write:favourites%20write:notifications%20read:statuses%20read:lists";
const auto redirect_uri = "urn:ietf:wg:oauth:2.0:oob";

void make_new_account(const std::string& accountname)
{
    print_logger<logtype::normal> pl;
    auto useraccount = options.select_account(accountname);

    // see: https://docs.joinmastodon.org/api/authentication/

    // if no user was found, make a new one
    if (useraccount == nullptr)
    {
        pl << "Creating new account for " << accountname << "\n";
        auto parsed = parse_account_name(accountname);
        if (!parsed.has_value())
        {
            pl << "Could not parse a username and instance name from: " << accountname << ". It should look like: username@instance.url\n";
            return;
        }
        // make a new account
        user_options& newuser = options.add_new_account(accountname);
        newuser.set_option(user_option::accountname, parsed->username);
        newuser.set_option(user_option::instanceurl, parsed->instance);
        useraccount = &newuser;
    }
    else
    {
        pl << "Existing user found.\n";
    }
    

    // register the application with mastodon if needed
    auto client_id = useraccount->get_option(user_option::clientid);
    auto client_secret = useraccount->get_option(user_option::clientsecret);

    auto instanceurl = *useraccount->get_option(user_option::instanceurl);
    if (client_id == nullptr || client_secret == nullptr)
    {
        pl << "Registering app with " << instanceurl << '\n';
        auto r = cpr::Post(cpr::Url{make_api_url(instanceurl, "/api/v1/apps")},
                           cpr::Parameters{{"client_name", "msync"}, {"redirect_uris", redirect_uri}, {"scopes", scopes}, {"website", "https://github.com/kansattica/msync"}});

        if (r.error)
        {
            pl << "Could not register app with server. Responded with error code " << r.status_code << ": " << r.error.message << '\n';
            pl << "Please try again.\n";
            return;
        }

        json parsed = json::parse(r.text);
        useraccount->set_option(user_option::clientid, parsed["client_id"].get<std::string>());
        useraccount->set_option(user_option::clientsecret, parsed["client_secret"].get<std::string>());
        pl << "Registered!\n";
    }
    else
    {
        pl << "App already registered.\n";
    }

    if (client_id == nullptr)
        client_id = useraccount->get_option(user_option::clientid);

    auto authcode = useraccount->get_option(user_option::authcode);
    if (authcode == nullptr)
    {
        auto foundaccountname = *useraccount->get_option(user_option::accountname);
        pl << "Please open this URL in your browser:\n"
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

    auto access_token = useraccount->get_option(user_option::accesstoken);
    if (access_token != nullptr)
    {
        pl << "Your account is already registered! You're done!\n";
        return;
    }

    pl << "Getting access token from server with authorization code.\n";
    auto response = cpr::Post(cpr::Url{make_api_url(instanceurl, "/oauth/token")}, 
        cpr::Parameters{{"client_id", *client_id}, {"client_secret", *client_secret}, {"grant_type", "authorization_code"}, 
        {"code", *authcode}, {"redirect_uri", redirect_uri}});

    useraccount->set_option(user_option::authcode, "");
    if (response.error)
    {
        auto foundaccountname = *useraccount->get_option(user_option::accountname);
        pl << "Could not get access token from server. Authorization codes can only be used once, so it's been deleted and you should get another one.\n"
           << "Please open this URL in your browser:\n"
           << "https://" << instanceurl << "/oauth/authorize?response_type=code&client_id=" << *client_id
           << "&redirect_uri=" << redirect_uri << "&scope=" << urlscopes << '\n'
           << "Enter your authorization code like so:\n"
           << "msync config authcode <the authorization code from the site> -a " << foundaccountname << "@" << instanceurl << '\n'
           << "then run this again:\n"
           << "msync new -a " << foundaccountname << "@" << instanceurl << '\n';
        return;
    }

    json parsed = json::parse(response.text);
    useraccount->set_option(user_option::accesstoken, parsed["access_token"].get<std::string>());
    pl << "Done! You're ready to start using this account.\n";

    //don't need these any more once we're good
    useraccount->set_option(user_option::clientid, "");
    useraccount->set_option(user_option::clientsecret, "");
}