#ifndef GLOBAL_OPTIONS_HPP
#define GLOBAL_OPTIONS_HPP

#include "user_options.hpp"
#include <filesystem.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <algorithm>
#include <vector>
#include <variant>

#include <filesystem.hpp>

enum class select_account_error
{
	no_accounts,
	empty_name_many_accounts,
	ambiguous_prefix,
	bad_prefix
};

using select_account_result = std::variant<std::pair<const std::string, user_options>*, select_account_error>;

struct global_options
{
public:
	global_options(fs::path accounts_dir);

	std::pair<const std::string, user_options>& add_new_account(std::string name);
	select_account_result select_account(std::string_view name);
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