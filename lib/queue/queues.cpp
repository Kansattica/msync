#include "queues.hpp"

#include "queue_list.hpp"
#include <constants.hpp>
#include <filesystem.hpp>

void queue_post(const std::string& account, const std::vector<std::string>& filenames)
{
}

void enqueue(queues toenqueue, const std::string& account, const std::vector<std::string>& add)
{
    if (toenqueue == queues::post)
    {
        queue_post(account, add);
        return;
    }

    fs::path qfile = fs::current_path() / account;
    qfile /= (toenqueue == queues::fav ? Fav_Queue_Filename : Boost_Queue_Filename);

    queue_list toadd{qfile};
}