#include "sync_helpers.hpp"

#include <random>

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
