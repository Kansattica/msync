#ifndef _USER_OPTIONS_HPP_
#define _USER_OPTIONS_HPP_

#include <string>

#include "option_enums.hpp"
#include "option_file.hpp"
#include <filesystem.hpp>

struct user_options
{
public:
	user_options(fs::path toread) : user_directory(toread.parent_path()), backing(toread){};

	const std::string* try_get_option(user_option toget) const;
	const std::string& get_option(user_option toget) const;
	sync_settings get_sync_option(user_option toget) const;
	bool get_bool_option(user_option toget) const;

	const fs::path& get_user_directory() const;

	void set_option(user_option toset, std::string value);
	void set_option(user_option toset, list_operations value);
	void set_option(user_option toset, sync_settings value);
	void set_bool_option(user_option toset, bool value);


private:
	const fs::path user_directory;
	option_file backing;
};
#endif
