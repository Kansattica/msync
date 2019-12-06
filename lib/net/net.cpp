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

	// if we're not okay, try and get the error message
	// but this is only some kinds of error.
	// if Mastodon returns an error (and not nginx),
	// then response.error.message will be empty, but response.text will have important stuff in it
	// for example, if the user sends a bad post, you'll get a 422 and the error is in response.text

	if (!to_return.okay)
	{
		to_return.message = std::move(response.error.message);
	}

	if (to_return.message.empty())
		to_return.message = std::move(response.text);

	return to_return;
}

constexpr auto idempotency_key_header{ "Idempotency-Key" };
constexpr auto authorization_key_header{ "Authorization" };
net_response simple_post(const std::string_view url, const std::string_view access_token)
{
	// add rate limiting handling later
	return handle_response(
		cpr::Post(cpr::Url{ url },
			cpr::Header{ { idempotency_key_header, std::string{ ensure_small_string(url) } } ,
						 { authorization_key_header, make_bearer(access_token) } }
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

net_response upload_media(std::string_view url, std::string_view access_token, const fs::path& file, const std::string& description)
{
	return handle_response(
		cpr::Post(cpr::Url{ url },
			cpr::Header{ {authorization_key_header, make_bearer(access_token) } },
			cpr::Multipart{ { "description", description },
							{ "file", cpr::File{file.string()} } }
	));
}

void add_if_value(cpr::Payload& params, const char* key, const std::string& value)
{
	if (!value.empty())
		params.AddPair(cpr::Pair(key, value));
}

void add_array(cpr::Payload& params, const char* key, const std::vector<std::string>& values)
{
	for (const auto& value : values)
	{
		params.AddPair(cpr::Pair(key, value));
	}
}

net_response new_status(std::string_view url, std::string_view access_token, const status_params& params)
{
	cpr::Payload post_params{};
	add_if_value(post_params, "status", params.body);
	add_if_value(post_params, "spoiler_text", params.content_warning);
	add_if_value(post_params, "visibility", params.visibility);
	add_if_value(post_params, "in_reply_to_id", params.reply_to);

	// the mastodon api page doesn't mention this, but apparently this is a feature of rails
	// (that the other mastodon api libs use)
	// that lets you specify arrays with empty brackets like this
	add_array(post_params, "media_ids[]", params.attachment_ids);


	return handle_response(
		cpr::Post(cpr::Url{ url },
			cpr::Header{ { idempotency_key_header, std::to_string(params.idempotency_key) },
						 { authorization_key_header, make_bearer(access_token) } },
			std::move(post_params)));
}

void add_if_value(cpr::Parameters& params, const char* key, const std::string_view value)
{
	if (!value.empty())
		params.AddParameter(cpr::Parameter{ key, value });
}

net_response get_timeline_and_notifs(std::string_view url, std::string_view access_token, const timeline_params& params, unsigned int limit)
{
	cpr::Parameters query_params{ { "limit", std::to_string(limit) } };

	add_if_value(query_params, "min_id", params.min_id);
	add_if_value(query_params, "max_id", params.max_id);
	add_if_value(query_params, "since_id", params.since_id);

	return handle_response(
		cpr::Get(cpr::Url{ url },
			cpr::Header{ {authorization_key_header, make_bearer(access_token) } },
			std::move(query_params)
		)
	);
}
