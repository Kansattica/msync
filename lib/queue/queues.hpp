#ifndef QUEUES_HPP
#define QUEUES_HPP

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

// enqueue and dequeue mutate and move from the vector for efficiency
void enqueue(queues toenqueue, const fs::path& user_account_dir, std::vector<std::string>&& add);
void dequeue(queues todequeue, const fs::path& user_account_dir, std::vector<std::string>&& remove);

void clear(queues toclear, const fs::path& user_account_dir);

queue_list get(const fs::path& user_account_dir);

std::vector<std::string> print(const fs::path& user_account_dir);

#endif