#include "queues.hpp"

#include "queue_list.hpp"
#include <constants.hpp>
#include <filesystem.hpp>
#include <print_logger.hpp>
#include "../options/global_options.hpp"
#include <algorithm>


fs::path get_file_queue_directory(const std::string& account, bool create)
{
	if (!create)
		return "";
	return options.executable_location / account / File_Queue_Directory;
}

void unique_file_name(fs::path& path)
{
	unsigned int extensionint = 1;
	path += ".0";
	do
	{
		path.replace_extension(std::to_string(extensionint++));
	} while (fs::exists(path));
}

std::string queue_post(const fs::path& queuedir, const fs::path& postfile)
{
	print_logger pl;
	fs::create_directories(queuedir);

	if (!fs::exists(postfile))
	{
		pl << "Could not find " << postfile << ". Skipping.\n";
		return "";
	}

	if (!fs::is_regular_file(postfile))
	{
		pl << postfile << " is not a file. Skipping.\n";
		return "";
	}

	fs::path copyto = queuedir / postfile.filename();

	if (fs::exists(copyto))
	{
		pl << copyto << " already exists. " << postfile << " will be saved as ";
		unique_file_name(copyto);
		pl << copyto << '\n';
	}

	fs::copy(postfile, copyto);
	return copyto.filename().string();
}

queue_list open_queue(const queues to_open, const std::string& account)
{
	fs::path qfile = options.executable_location / account;
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

	const fs::path filequeuedir = get_file_queue_directory(account, toenqueue == queues::post);
	for (auto& id : add)
	{
		std::string queuethis = id;
		if (toenqueue == queues::post)
		{
			queuethis = queue_post(filequeuedir, id);
		}
		toaddto.queued.emplace_back(std::move(queuethis));
	}

	// consider looking for those "delete" guys, the ones with the - at the end, and having this cancel them out, 
	// but "unboost and reboost", for example, is a valid thing to want to do.
}

void dequeue_post(const fs::path &queuedir, const fs::path& filename)
{
	print_logger pl;
	if (!fs::remove(queuedir / filename))
	{
		pl << "Could not delete " << filename << ", could not find it in " << queuedir << '\n';
	}
}

void just_filename(std::string& path)
{
	path = fs::path(path).filename().string();
}

void dequeue(queues todequeue, const std::string& account, std::vector<std::string>&& toremove)
{
	queue_list toremovefrom = open_queue(todequeue, account);

	const fs::path filequeuedir = get_file_queue_directory(account, todequeue == queues::post);
	for (auto it = toremove.begin(); it != toremove.end(); )
	{
		if (todequeue == queues::post)
			just_filename(*it);
		
		// if the item is in the queue, remove it form the queue and from the toremove vector
		// if this gets to be a performance bottleneck (since it's an n^2 algorithm), make an unordered_set from toremove or sort and do a binary search
		auto inqueue = std::find(toremovefrom.queued.begin(), toremovefrom.queued.end(), *it);
		if (inqueue != toremovefrom.queued.end()) // if we found the thing in the queue, remove from both
		{
			if (todequeue == queues::post)
			{
				dequeue_post(filequeuedir, *it);
			}
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