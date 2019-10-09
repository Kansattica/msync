#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <string_view>
#include <vector>

enum class queues
{
    fav,
    boost,
    post
};

void enqueue(queues toenqueue, const std::string_view account, const std::vector<std::string>& add);

// dequeue mutates and moves from the vector for efficiency
void dequeue(queues todequeue, const std::string_view account, std::vector<std::string>&& remove);
void clear(queues toclear, const std::string_view account);

std::vector<std::string> print(queues toprint, const std::string_view account);

#endif