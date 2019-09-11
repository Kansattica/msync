#include "newaccount.hpp"
#include "../lib/options/global_options.hpp"
#include "../lib/util/util.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print_logger.hpp>
#include <string>

using json = nlohmann::json;

const auto scopes = "write:favourites write:notifications read:statuses read:lists";
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

    // register the application with mastodon if needed
    auto client_id = useraccount->get_option(user_option::clientid);
    auto client_secret = useraccount->get_option(user_option::clientsecret);

    if (client_id == nullptr || client_secret == nullptr)
    {
        auto r = cpr::Post(cpr::Url{make_api_url(*useraccount->get_option(user_option::instanceurl), "/api/v1/apps")},
                           cpr::Parameters{{"client_name", "msync"}, {"redirect_uris", redirect_uri}, {"scopes", scopes}, {"website", "https://github.com/kansattica/msync"}});

        if (r.error)
        {
            pl << "Could not register app with server. Responded with error code " << r.status_code << ": " << r.error.message << '\n';
            pl << "Please try again.\n";
            return;
        }

        json parsed = r.text;
        useraccount->set_option(user_option::clientid, parsed["client_id"].get<std::string>());
        useraccount->set_option(user_option::clientsecret, parsed["client_secret"].get<std::string>());
    }
}