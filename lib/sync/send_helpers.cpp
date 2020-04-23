#include "send_helpers.hpp"

#include <random>
#include <algorithm>
#include <unordered_map>

#include "../postfile/outgoing_post.hpp"

std::string paramaterize_url(const std::string_view before, const std::string_view middle, const std::string_view after)
{
	std::string toreturn{ before };
	toreturn.reserve(toreturn.size() + middle.size() + after.size());
	return toreturn.append(middle).append(after);
}

std::mt19937_64 make_random_engine()
{
	// random_device produces an unsigned int (32 bits), but the mersenne twister wants to be seeded with a 64-bit value,
	// so take two seeds and combine them like this
	// this is overkill for what msync uses the mersenne twister for, which is to simply generate a sequence of
	// idempotency keys that won't repeat within or between runs, but if you're gonna do something, might as well do it right.
	// I think the real proper thing to do would be to use seedseq, but I haven't found any real documentation on
	// how to use it correctly.
	// if you're on a system where this isn't true (say, your unsigned int is 16 bits), you should only have to make
	// two more calls to rd().
	static_assert(sizeof(std::random_device::result_type) * 2 == sizeof(std::mt19937_64::result_type));
	std::random_device rd;
	std::mt19937_64::result_type seed = rd();
	seed = (seed << 32) | rd();
	return std::mt19937_64(seed);
}

uint_fast64_t random_number()
{
	static auto twister = make_random_engine();
	return twister();
}

std::vector<attachment> make_attachments(std::vector<std::string>&& attachments, std::vector<std::string>&& descriptions)
{
	descriptions.resize(attachments.size());
	std::vector<attachment> toreturn;
	toreturn.reserve(attachments.size());

	for (size_t i = 0; i < attachments.size(); i++)
	{
		toreturn.push_back(attachment{ std::move(attachments[i]), std::move(descriptions[i]) });
	}

	return toreturn;
}

static std::unordered_map<std::string, std::string> threaded_ids;

// the idea here is that posts can optionally have some local ID. 
// if another post's reply_to_id is set to one of those, then fix it up so that 
// that post is a reply to the first one.

void store_thread_id(std::string msync_id, std::string remote_server_id)
{
	// if another post uses the same key ID, replace it
	// this is mostly to make the tests work, though it is also closer to what you might expect

	// I considered scoping the threaded_ids to the process_posts functions, but it's probably good to
	// let people thread across accounts, even though we currently can't really guarantee an order, so hm
	threaded_ids.insert_or_assign(std::move(msync_id), std::move(remote_server_id));
}

file_status_params read_params(const fs::path& path)
{
	readonly_outgoing_post post{ path };

	file_status_params toreturn;
	toreturn.idempotency_key = random_number();
	toreturn.attachments = make_attachments(std::move(post.parsed.attachments), std::move(post.parsed.descriptions));
	toreturn.body = std::move(post.parsed.text);
	toreturn.content_warning = std::move(post.parsed.content_warning);
	toreturn.reply_to = std::move(post.parsed.reply_to_id);
	toreturn.reply_id = std::move(post.parsed.reply_id);

	if (!toreturn.reply_to.empty())
	{
		const auto val = threaded_ids.find(toreturn.reply_to);
		if (val != threaded_ids.end()) 
		{
			toreturn.reply_to = val->second;
			toreturn.okay = !toreturn.reply_to.empty();
		}
	}

	toreturn.visibility = post.parsed.visibility_string();

	return toreturn;
}
