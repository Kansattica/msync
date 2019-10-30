#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <string_view>
#include <vector>

#include "queue_list.hpp"

enum class queues
{
    fav,
    boost,
    post
};

fs::path get_file_queue_directory(std::string_view account);

void enqueue(queues toenqueue, const std::string_view account, const std::vector<std::string>& add);

// dequeue mutates and moves from the vector for efficiency
void dequeue(queues todequeue, const std::string_view account, std::vector<std::string>&& remove);
void clear(queues toclear, const std::string_view account);

queue_list get(queues toget, const std::string_view account);

std::vector<std::string> print(queues toprint, const std::string_view account);

#endif