#ifndef _SYNC_HELPERS_
#define _SYNC_HELPERS_

#include <string_view>
#include <optional>

#include <filesystem.hpp>

#include "../net_interface/net_interface.hpp"

bool should_undo(std::string_view& id);

std::string paramaterize_url(std::string_view before, std::string_view middle, std::string_view after);

struct attachment
{
	fs::path file;
	std::string description;
};

struct file_status_params : public status_params
{
	std::vector<attachment> attachments;
};

file_status_params read_params(const fs::path& path);
#endif