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
	const std::string& to_append = [to_open]() {
		switch (to_open)
		{
		case queues::fav:
			return Fav_Queue_Filename;
			break;
		case queues::boost:
			return Boost_Queue_Filename;
			break;
		case queues::post:
			return Post_Queue_Filename;
			break;
		}
	}();

	return queue_list{ qfile / to_append };
}

void enqueue(const queues toenqueue, const std::string& account, const std::vector<std::string>& add)
{
	queue_list toaddto = open_queue(toenqueue, account);

	for (auto& id : add)
	{
		toaddto.queued.emplace_back(id);
	}

	if (toenqueue == queues::post)
	{
		queue_post(account, add);
	}

	// consider looking for those "delete" guys, the ones with the - at the end, and having this cancel them out, 
	// but "unboost and reboost", for example, is a valid thing to want to do.

	return;
}

void dequeue(queues todequeue, const std::string& account, const std::vector<std::string>& toremove)
{
	queue_list toremovefrom = open_queue(todequeue, account);

	std::vector<std::string> todel;

	for (auto it = toremovefrom.queued.begin(); it != toremovefrom.queued.end();)
	{
		// this is slow. if it becomes an issue, consider sorting toremove first and doing
		// a binary search or making an unordered_set from the contents of toremove.
		if (std::any_of(toremove.begin(), toremove.end(), [it](auto& tor) {
			return tor == *it;
			}))
		{
			it = toremovefrom.queued.erase(it);
		}
		else
		{
			todel.push_back(*it);
			++it;
		}
	}

	//basically, if a thing isn't in the queue, enqueue removing that thing. unboosting, unfaving, deleting a post

	for (auto& queuedel : todel)
	{
		queuedel.push_back('-');
		toremovefrom.queued.emplace_back(std::move(queuedel));
	}
}

void clear(queues toclear, const std::string& account)
{
	queue_list clearthis = open_queue(toclear, account);

	clearthis.queued.clear();
}