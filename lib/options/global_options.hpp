#ifndef _GLOBAL_OPTIONS_HPP_
#define _GLOBAL_OPTIONS_HPP_

#include "user_options.hpp"
#include <filesystem.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <algorithm>
#include <vector>

#include <filesystem.hpp>

struct global_options
{
public:
	global_options(fs::path accounts_dir);

	std::pair<const std::string, user_options>& add_new_account(std::string name);
	std::pair<const std::string, user_options>* select_account(std::string_view name);
	std::vector<std::string_view> all_accounts() const;

	template <typename Callable>
	void foreach_account(Callable c)
	{
		std::for_each(accounts.begin(), accounts.end(), c);
	}
private:
	std::vector<std::pair<const std::string, user_options>> accounts;
	const fs::path accounts_directory;
};
#endif