#include "sync_helpers.hpp"

#include <random>
#include <algorithm>

#include "../postfile/outgoing_post.hpp"

bool should_undo(std::string_view& id)
{
	if (id.back() == '-')
	{
		id.remove_suffix(1);
		return true;
	}

	return false;
}

std::string paramaterize_url(const std::string_view before, const std::string_view middle, const std::string_view after)
{
	std::string toreturn{ before };
	toreturn.reserve(toreturn.size() + middle.size() + after.size());
	return toreturn.append(middle).append(after);
}

std::mt19937_64 make_random_engine()
{
	std::random_device rd;
	return std::mt19937_64(rd());
}

uint64_t random_number()
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

file_status_params read_params(const fs::path& path)
{
	readonly_outgoing_post post{ path };

	file_status_params toreturn;
	toreturn.idempotency_key = random_number();
	toreturn.attachments = make_attachments(std::move(post.parsed.attachments), std::move(post.parsed.descriptions));
	toreturn.body = std::move(post.parsed.text);
	toreturn.content_warning = std::move(post.parsed.content_warning);
	toreturn.reply_to = std::move(post.parsed.reply_to_id);
	toreturn.visibility = post.parsed.visibility_string();

	return toreturn;
}