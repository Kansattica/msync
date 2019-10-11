#include "net.hpp"

#include <cpr/cpr.h>

net_response simple_post(const std::string_view url, const std::string_view access_token)
{
	static const std::string key_header{ "Idempotency-Key" };
	auto response = cpr::Post(cpr::Url{ url }, cpr::Authentication{ "Bearer", access_token }, cpr::Header{ {key_header, std::string{url} } });

	// add rate limiting handling later

	net_response toreturn;

	toreturn.retryable_error = response.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT || (response.status_code >= 500 && response.status_code < 600);
	toreturn.okay = !response.error;

	return toreturn;
}

