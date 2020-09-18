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

	to_return.status_code = response.status_code;


	to_return.retryable_error = response.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT || (response.status_code >= 500 && response.status_code < 600);

	// Some timeouts look like CONNECTION_FAILUREs and should be retried.
	if (!to_return.retryable_error && response.error.code == cpr::ErrorCode::CONNECTION_FAILURE)
	{
		constexpr std::string_view timed_out {"timed out"};
		to_return.retryable_error = response.error.message.find(timed_out) != std::string::npos;
	}

	// I think response.error refers to whether curl itself reported an error, as opposed to the remote server
	to_return.okay = !response.error && response.status_code >= 200 && response.status_code < 300;

	// https://docs.joinmastodon.org/api/rate-limits/
	// Waiting if X-RateLimit-Remaining is 0 is a good idea.
	// It means we wind up spuriously waiting if we run out of ratelimit but don't need to make any more requests.
	// but msync rarely knows it has to make zero more requests.
	// The only case would be if it finished sending all its boosts, favs, and posts and didn't sync timelines or notifications at all,
	// because the only way to know you're done receiving is to get something back with fewer posts than you asked for.
	// Which is pretty unlikely, since to hit the rate limit while just sending, you'd have to do one of:
	// - delete upwards of 30 posts or boosts
	// - upload 30 media attachments
	// - make over 300 posts or calls to other API endpoints
	// so the ratelimit zero thing is probably a good idea.
	if (response.status_code == 429 || (to_return.okay && response.header["X-RateLimit-Remaining"][0] == '0'))
	{
		to_return.message = std::move(response.header["X-RateLimit-Reset"]);
		to_return.retryable_error = true;
		to_return.okay = false;
		return to_return;
	}

	// if we're not okay, try and get the error message
	// but this is only some kinds of error.
	// if Mastodon returns an error (and not nginx or curl),
	// then response.error.message will be empty, but response.text will have important stuff in it
	// for example, if the user sends a bad post, you'll get a 422 and the error is in response.text

	if (!to_return.okay)
		to_return.message = std::move(response.error.message);

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
			// cpr::File won't take a wchar string on Windows or a fs::path, so I think my best bet is to hope that .string()
			// does whatever it does, and then CPR passes that on to the underlying filesystem unchanged and things will work out.
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

void add_array(cpr::Parameters& params, const char* key, const std::vector<std::string_view>& values)
{
	for (const auto& value : values)
	{
		params.AddParameter(cpr::Parameter{ key, value });
	}
}

net_response get_timeline_and_notifs(std::string_view url, std::string_view access_token, const timeline_params& params, unsigned int limit)
{
	cpr::Parameters query_params{ { "limit", std::to_string(limit) } };

	add_if_value(query_params, "min_id", params.min_id);
	add_if_value(query_params, "max_id", params.max_id);
	add_if_value(query_params, "since_id", params.since_id);

	if (params.exclude_notifs != nullptr) { add_array(query_params, "exclude_types[]", *params.exclude_notifs); }

	return handle_response(
		cpr::Get(cpr::Url{ url },
			cpr::Header{ {authorization_key_header, make_bearer(access_token) } },
			std::move(query_params)
		)
	);
}
