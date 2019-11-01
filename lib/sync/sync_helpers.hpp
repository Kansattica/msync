#ifndef _SYNC_HELPERS_
#define _SYNC_HELPERS_

#include <string_view>

bool should_undo(std::string_view& id);

std::string paramaterize_url(const std::string_view before, const std::string_view middle, const std::string_view after);

uint64_t random_number();
#endif