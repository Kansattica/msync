#ifndef _SYNC_HELPERS_
#define _SYNC_HELPERS_

#include <string_view>
#include <optional>

#include <filesystem.hpp>

#include "../net_interface/net_interface.hpp"

bool should_undo(std::string_view& id);

std::string paramaterize_url(std::string_view before, std::string_view middle, std::string_view after);

void store_thread_id(std::string msync_id, std::string remote_server_id);

struct attachment
{
	fs::path file;
	std::string description;
};

struct file_status_params : public status_params
{
	std::vector<attachment> attachments;
	std::string reply_id;
	bool okay = true;
};

file_status_params read_params(const fs::path& path);

template <typename Stream>
void print_truncated_string(std::string_view toprint, Stream& str)
{
	static constexpr int max_length = 60;
	bool truncated = false;
	if (toprint.size() > max_length)
	{
		toprint.remove_suffix(toprint.size() - max_length);
		truncated = true;
	}

	if (!toprint.empty() && toprint.back() == '\n')
		toprint.remove_suffix(1);

	str << toprint;
	if (truncated)
		str << "...";

}
#endif
