#include "new_account.hpp"

#include "../lib/util/util.hpp"
#include "../lib/options/user_options.hpp"    // for user_options

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print_logger.hpp>

#include <string>
#include <utility>                            // for pair

using json = nlohmann::json;

constexpr auto scopes = "write:favourites write:media write:statuses read:notifications read:statuses write:bookmarks read:bookmarks";
constexpr auto urlscopes = "write:favourites%20write:media%20write:statuses%20read:notifications%20read:statuses%20write:bookmarks%20read:bookmarks";
constexpr auto redirect_uri = "urn:ietf:wg:oauth:2.0:oob";

std::string make_clean_accountname(const std::string& username, const std::string& instance)
{
	std::string to_return;
	to_return.reserve(username.size() + instance.size() + 1);
	return to_return.append(username).append(1, '@').append(instance);
}

void make_new_account(const std::string& accountname, global_options& options)
{
	auto useraccountresult = options.select_account(accountname);

	auto useraccountpair = useraccountresult.index() == 0 ? std::get<0>(useraccountresult) : nullptr;

	// see: https://docs.joinmastodon.org/api/authentication/

	// if no user was found, make a new one
	if (useraccountpair == nullptr)
	{
		if (accountname.empty())
		{
			pl() << "Please specify the account you'd like to add to msync. For example:\n"
				"msync new --account username@instance.egg";
			return;
		}

		pl() << "Creating new account for " << accountname << ".\n";
		const auto parsed = parse_account_name(accountname);
		if (!parsed.has_value())
		{
			pl() << "Could not parse a username and instance name from: " << accountname << ". It should look like: username@instance.url";
			return;
		}
		// make a new account
		useraccountpair = &(options.add_new_account(make_clean_accountname(parsed->username, parsed->instance)));
		useraccountpair->second.set_option(user_option::account_name, parsed->username);
		useraccountpair->second.set_option(user_option::instance_url, parsed->instance);
		useraccountpair->second.set_option(user_option::file_version, "1");
	}
	else
	{
		pl() << "Existing user found.\n";
	}
	
	auto& useraccount = useraccountpair->second;

	if (useraccount.try_get_option(user_option::access_token) != nullptr)
	{
		pl() << "This user is already registered with msync. You don't have to do anything else here.\n"
			"If you'd like to go through the setup process again, delete the following folder in msync_accounts and try again:\n"
			<< useraccountpair->first;
		return;
	}

	// register the application with mastodon if needed
	auto client_id = useraccount.try_get_option(user_option::client_id);
	const auto client_secret = useraccount.try_get_option(user_option::client_secret);

	const auto& instanceurl = useraccount.get_option(user_option::instance_url);
	if (client_id == nullptr || client_secret == nullptr)
	{
		pl() << "Registering app with " << instanceurl << ".\n";
		const auto r = cpr::Post(cpr::Url{make_api_url(instanceurl, "/api/v1/apps")},
						   cpr::Payload{{"client_name", "msync"}, {"redirect_uris", redirect_uri}, {"scopes", scopes}, {"website", "https://github.com/kansattica/msync"}});

		if (r.error || r.status_code != 200)
		{
			pl() << "Could not register app with server. Responded with error code " << r.status_code << ": " << r.error.message << ".\n"
			"Response body: " << r.text << "\n"
			"Please double check your instance URL, ensure you're connected to the internet, and try again.";
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
		pl() << "Please open this URL in your browser:\n\n"
		   "https://" << instanceurl << "/oauth/authorize?response_type=code&client_id=" << *client_id
		   << "&redirect_uri=" << redirect_uri << "&scope=" << urlscopes << "\n\n"
		   "Log in, then copy and enter your authorization code like so:\n\n"
		   "msync config auth_code <the authorization code from the site> --account " << foundaccountname << '@' << instanceurl << "\n\n"
		   "then run this again:\n\n"
		   "msync new -a " << foundaccountname << '@' << instanceurl << "\n\n"
		   "You can shorten the username part, as long as msync can figure out which registered account you're talking about.\n"
		   "If this is your only account, you can leave the -a part off altogether.";
		return;
	}

	const auto access_token = useraccount.try_get_option(user_option::access_token);
	if (access_token != nullptr)
	{
		pl() << "Your account is already registered! You're done!";
		return;
	}

	pl() << "Getting access token from server with authorization code.\n";
	const auto response = cpr::Post(cpr::Url{make_api_url(instanceurl, "/oauth/token")},
		cpr::Payload{{"client_id", *client_id}, {"client_secret", *client_secret}, {"grant_type", "authorization_code"},
		{"code", *authcode}, {"redirect_uri", redirect_uri}});

	useraccount.set_option(user_option::auth_code, "");
	if (response.error)
	{
		const auto foundaccountname = *useraccount.try_get_option(user_option::account_name);
		pl() << "Could not get access token from server. Authorization codes can only be used once, so it's been deleted and you should get another one.\n"
		   "Please open this URL in your browser:\n"
		   "https://" << instanceurl << "/oauth/authorize?response_type=code&client_id=" << *client_id
		   << "&redirect_uri=" << redirect_uri << "&scope=" << urlscopes << "\n"
		   "Enter your authorization code like so:\n"
		   "msync config authcode <the authorization code from the site> -a " << foundaccountname << '@' << instanceurl << "\n"
		   "then run this again:\n"
		   "msync new -a " << foundaccountname << '@' << instanceurl;
		return;
	}

	const json parsed = json::parse(response.text);
	useraccount.set_option(user_option::access_token, parsed["access_token"].get<std::string>());
	pl() << "Done! You're ready to start using this account."; //main will add its own newline before it exits.

	//don't need these any more once we're good
	useraccount.set_option(user_option::client_id, "");
	useraccount.set_option(user_option::client_secret, "");
}
