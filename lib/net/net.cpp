#include "net.hpp"

#include <cpr/cpr.h>
#include <string>
#include <utility>

#include <filesystem.hpp>

std::string_view ensure_small_string(const std::string_view sv)
{
	// I want to ensure that, when a string is constructed from this, it's cheap to make
	// I believe gcc, clang, and vs will all short string optimize a string that's 20 characters or less
	// this is fine for the idempotency header, which only need be unique per type of request
	// this only works for urls that end in the post ID, gotta do something else for statuses.
	// also, we want to take the end because that's where the status and post ID are
	const size_t start_at = sv.size() < 20 ? 0 : sv.size() - 20;
	return sv.substr(start_at);
}
std::string make_bearer(const std::string_view access_token)
{
	return std::string{ "Bearer " }.append(access_token);
}

net_response handle_response(cpr::Response&& response)
{
	net_response to_return;

	to_return.retryable_error = response.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT || (response.status_code >= 500 && response.status_code < 600);

	// I don't think we can trust response.error, it says OK even if the status code is 400 something
	to_return.okay = !response.error && response.status_code >= 200 && response.status_code < 300;

	if (to_return.okay)
		to_return.message = std::move(response.text);
	else
		to_return.message = std::move(response.error.message);

	return to_return;
}

constexpr auto idempotency_key_header{ "Idempotency-Key" };
constexpr auto authorization_key_header{ "Authorization" };
net_response simple_post(const std::string_view url, const std::string_view access_token)
{
	// add rate limiting handling later
	return handle_response(
		cpr::Post(cpr::Url{ url },
			cpr::Header{ {idempotency_key_header, std::string{ ensure_small_string(url) } } ,
						 {authorization_key_header, make_bearer(access_token) } }
		)
	);
}

net_response simple_delete(const std::string_view url, const std::string_view access_token)
{
	// add rate limiting handling later
	return handle_response(
		cpr::Delete(cpr::Url{ url },
			cpr::Header{ {authorization_key_header, make_bearer(access_token) } })
	);
}

void add_if_value(cpr::Parameters& params, const char* key, std::string&& value)
{
	if (!value.empty())
		params.AddParameter(cpr::Parameter(key, std::move(value)));
}

net_response new_status(std::string_view url, std::string_view access_token, status_params params)
{
	std::string idempotency_key = params.body;

	cpr::Parameters post_params;
	add_if_value(post_params, "status", std::move(params.body));
	add_if_value(post_params, "spoiler_text", std::move(params.content_warning));
	add_if_value(post_params, "visibility", std::move(params.visibility));
	add_if_value(post_params, "in_reply_to_id", std::move(params.reply_to));

	// gotta handle attachments

	return handle_response(
		cpr::Post(cpr::Url{ url },
			cpr::Header{ { idempotency_key_header, std::move(idempotency_key) },
						 { authorization_key_header, make_bearer(access_token) } },
						 std::move(post_params)));
}
