#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <vector>

enum class queues
{
    fav,
    boost,
    post
};

void enqueue(queues toenqueue, const std::string& account, const std::vector<std::string>& add);

// dequeue mutates and moves from the vector for efficiency
void dequeue(queues todequeue, const std::string& account, std::vector<std::string>&& remove);
void clear(queues toclear, const std::string& account);

#endif