#include "queues.hpp"

#include "queue_list.hpp"
#include <constants.hpp>
#include <filesystem.hpp>

#include <algorithm>

void queue_post(const std::string& account, const std::vector<std::string>& filenames)
{
}

queue_list open_queue(const queues to_open, const std::string& account)
{
    fs::path qfile = fs::current_path() / account;
    const std::string* to_append;
    switch (to_open)
    {
    case queues::fav:
        to_append = &Fav_Queue_Filename;
        break;
    case queues::boost:
        to_append = &Boost_Queue_Filename;
        break;
    case queues::post:
        to_append = &Post_Queue_Filename;
        break;
    }
    return queue_list{qfile / *to_append};
}

void enqueue(const queues toenqueue, const std::string& account, const std::vector<std::string>& add)
{
    if (toenqueue == queues::post)
    {
        queue_post(account, add);
        return;
    }

    queue_list toadd = open_queue(toenqueue, account);

    for (auto& id : add)
    {
        toadd.parsed.emplace_back(id);
    }
}