#ifndef QUEUES_HPP
#define QUEUES_HPP

#include <string>
#include <vector>

#include "queue_list.hpp"
#include <filesystem.hpp>

// enqueue and dequeue mutate the vector for efficiency
// note that they're passed by value
void enqueue(api_route toenqueue, const fs::path& user_account_dir, std::vector<std::string> add);
void dequeue(api_route todequeue, const fs::path& user_account_dir, std::vector<std::string> remove);

void clear(api_route toclear, const fs::path& user_account_dir);

queue_list get(const fs::path& user_account_dir);

std::vector<std::string> print(const fs::path& user_account_dir);

#endif
