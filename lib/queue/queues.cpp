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

void dequeue(queues todequeue, const std::string& account, std::vector<std::string>&& toremove)
{
	queue_list toremovefrom = open_queue(todequeue, account);

	for (auto it = toremove.begin(); it != toremove.end(); )
	{
		// if the item is in the queue, remove it form the queue and from the toremove vector
		// if this gets to be a performance bottleneck (since it's an n^2 algorithm), make an unordered_set from toremove
		auto inqueue = std::find(toremovefrom.queued.begin(), toremovefrom.queued.end(), *it);
		if (inqueue != toremovefrom.queued.end()) // if we found the thing in the queue, remove from both
		{
			toremovefrom.queued.erase(inqueue);
			it = toremove.erase(it);
		}
		else
		{
			++it;
		}
	}

	//basically, if a thing isn't in the queue, enqueue removing that thing. unboosting, unfaving, deleting a post
	//consider removing duplicate removes?

	for (auto& queuedel : toremove)
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