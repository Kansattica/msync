#include "net.hpp"

#include <cpr/cpr.h>
#include <string>

std::string_view ensure_small_string(const std::string_view sv)
{
	// I want to ensure that, when a string is constructed from this, it's cheap to make
	// I believe gcc, clang, and vs will all short string optimize a string that's 20 characters or less
	// this is fine for the idempotency header, which only need be unique per type of request
	// this only works for urls that end in the post ID, gotta do something else for statuses.
	// also, we want to take the end because that's where the status and post ID are
	int start_at = sv.size() < 20 ? 0 : sv.size() - 20;
	return sv.substr(start_at);
}
std::string make_bearer(const std::string_view access_token)
{
	return std::string{ "Bearer " }.append(access_token);
}

net_response simple_post(const std::string_view url, const std::string_view access_token)
{
	static const std::string key_header{ "Idempotency-Key" };
	auto response = cpr::Post(cpr::Url{ url },
		cpr::Header{ {key_header, std::string{ ensure_small_string(url) } } },
		cpr::Header{ {"Authorization", make_bearer(access_token) } });

	// add rate limiting handling later

	net_response toreturn;

	toreturn.retryable_error = response.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT || (response.status_code >= 500 && response.status_code < 600);

	// I don't think we can trust response.error, it says OK even if the status code is 400 something
	toreturn.okay = !response.error && response.status_code >= 200 && response.status_code < 300;

	if (toreturn.okay)
		toreturn.message = std::move(response.text);
	else
		toreturn.message = std::move(response.error.message);

	return toreturn;
}
