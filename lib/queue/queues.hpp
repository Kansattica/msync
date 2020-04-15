#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <vector>

#include "queue_list.hpp"

enum class queues
{
	fav,
	boost,
	post
};

fs::path get_file_queue_directory(const std::string& account);

// enqueue and dequeue mutate and move from the vector for efficiency
void enqueue(queues toenqueue, const std::string& account, std::vector<std::string>&& add);
void dequeue(queues todequeue, const std::string& account, std::vector<std::string>&& remove);

void clear(queues toclear, const std::string& account);

queue_list get(const std::string& account);

std::vector<std::string> print(const std::string& account);

#endif