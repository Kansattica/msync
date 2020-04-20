#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <vector>

#include "queue_list.hpp"
#include <filesystem.hpp>

enum class queues
{
	fav,
	boost,
	post
};

fs::path get_file_queue_directory(const fs::path& user_account_dir);

// enqueue and dequeue mutate and move from the vector for efficiency
void enqueue(queues toenqueue, const fs::path& user_account_dir, std::vector<std::string>&& add);
void dequeue(queues todequeue, const fs::path& user_account_dir, std::vector<std::string>&& remove);

void clear(queues toclear, const fs::path& user_account_dir);

queue_list get(const fs::path& user_account_dir);

std::vector<std::string> print(const fs::path& user_account_dir);

#endif