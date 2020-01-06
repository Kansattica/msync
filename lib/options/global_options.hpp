#ifndef _GLOBAL_OPTIONS_HPP_
#define _GLOBAL_OPTIONS_HPP_

#include "user_options.hpp"
#include <filesystem.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <algorithm>

struct global_options
{
public:
    const fs::path account_directory_location;

	global_options();

	std::pair<const std::string, user_options>& add_new_account(std::string name);
	std::pair<const std::string, user_options>* select_account(std::string_view name);
	void clear_accounts();
	std::vector<std::string_view> all_accounts() const;

	template <typename Callable>
	void foreach_account(Callable c)
	{
		std::for_each(accounts.begin(), accounts.end(), c);
	}
private:
	std::vector<std::pair<const std::string, user_options>> accounts;
};

global_options& options();
#endif