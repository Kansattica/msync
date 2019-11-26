#include "queues.hpp"

#include <constants.hpp>
#include <filesystem.hpp>
#include <system_error>
#include <print_logger.hpp>
#include "../options/global_options.hpp"
#include "../postfile/outgoing_post.hpp"
#include <algorithm>
#include <msync_exception.hpp>


fs::path get_file_queue_directory(std::string_view account)
{
	return options().account_directory_location / account / File_Queue_Directory;
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

void queue_attachments(const fs::path& postfile)
{
	outgoing_post post{ postfile };
	std::error_code err;
	for (auto& attach : post.parsed.attachments)
	{
		const auto attachpath = fs::canonical(attach, err);

		if (err)
		{
			pl() << "Error finding file: " << attach << "\nError:\n" << err << "\nSkipping.\n";
			continue;
		}

		if (!fs::is_regular_file(attachpath))
		{
			pl() << attachpath << " is not a regular file. Skipping.\n";
			continue;
		}

		attach = attachpath.string();

		err.clear();
	}
}

std::string queue_post(const fs::path& queuedir, const fs::path& postfile)
{
	fs::create_directories(queuedir);

	if (!fs::exists(postfile))
	{
		pl() << "Could not find " << postfile << ". Skipping.\n";
		return "";
	}

	if (!fs::is_regular_file(postfile))
	{
		pl() << postfile << " is not a file. Skipping.\n";
		return "";
	}

	fs::path copyto = queuedir / postfile.filename();

	if (fs::exists(copyto))
	{
		pl() << copyto << " already exists. " << postfile << " will be saved as ";
		unique_file_name(copyto);
		pl() << copyto << '\n';
	}

	fs::copy(postfile, copyto);

	queue_attachments(copyto);

	return copyto.filename().string();
}

queue_list open_queue(const queues to_open, const std::string_view account)
{
	fs::path qfile = options().account_directory_location / account;
	fs::create_directories(qfile);
	const std::string_view to_append = [to_open]() {
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
		default:
			throw msync_exception("whoops, this shouldn't happen.");
		}
	}();

	return queue_list{ qfile / to_append };
}

void enqueue(const queues toenqueue, const std::string_view account, const std::vector<std::string>& add)
{
	queue_list toaddto = open_queue(toenqueue, account);

	const fs::path filequeuedir = toenqueue == queues::post ? get_file_queue_directory(account) : "";
	for (const auto& id : add)
	{
		std::string queuethis = id;
		if (toenqueue == queues::post)
		{
			queuethis = queue_post(filequeuedir, id);
		}
		toaddto.parsed.push_back(std::move(queuethis));
	}

	// consider looking for those "delete" guys, the ones with the - at the end, and having this cancel them out, 
	// but "unboost and reboost", for example, is a valid thing to want to do.
}

void dequeue_post(const fs::path &queuedir, const fs::path& filename)
{
	if (!fs::remove(queuedir / filename))
	{
		pl() << "Could not delete " << filename << ", could not find it in " << queuedir << '\n';
	}
}

void dequeue(queues todequeue, const std::string_view account, std::vector<std::string>&& toremove)
{
	queue_list toremovefrom = open_queue(todequeue, account);

	if (todequeue == queues::post)
	{
		// trim path names 
		std::transform(toremove.begin(), toremove.end(), toremove.begin(), [](const auto& path) { return fs::path(path).filename().string(); });
	}

	// stable_partition is O(n) (assuming it can allocate a temporary buffer)
	// but doing a O(n) find call for each one makes it O(n^2)
	// sorting toremove and doing a binary search on it might be faster, 
	// but that's an O(n lg n) sort and then an O(n lg n) partition-with-find.
	// if the problem gets big enough, an unordered_set made from the elements of toremove would help
	// but I think that's overkill for the small lists this program will likely deal with.

	// put the ones to keep first, and the ones to remove last
	const auto removefrom_pivot = std::stable_partition(toremovefrom.parsed.begin(), toremovefrom.parsed.end(),
		[&toremove](const auto& id) { return std::find(toremove.begin(), toremove.end(), id) == toremove.end(); });


	// put the ones that'll be removed from the queue first and the rest after 

	// all the ones in toremovefrom that are also in toremove are at the end, so we only have to check those.
	// again, partitioning is O(n) and std::find is O(n), so this is another n^2 algorithm.
	// but these should be relatively small and so I highly doubt it's going to matter. If it does,
	// I could copy the things that'll be removed from the queue to an unordered set. Again, overkill for this.
	const auto toremove_pivot = std::stable_partition(toremove.begin(), toremove.end(),
		[&toremovefrom, removefrom_pivot](const auto& id) { return std::find(removefrom_pivot, toremovefrom.parsed.end(), id) != toremovefrom.parsed.end(); });

	if (todequeue == queues::post)
	{
		const fs::path filequeuedir = get_file_queue_directory(account);
		std::for_each(toremove.begin(), toremove_pivot,
			[&filequeuedir](const auto& filepath) { dequeue_post(filequeuedir, filepath); });
	}

	toremovefrom.parsed.erase(removefrom_pivot, toremovefrom.parsed.end());

	//basically, if a thing isn't in the queue, enqueue removing that thing. unboosting, unfaving, deleting a post
	//consider removing duplicate removes?
	std::for_each(toremove_pivot, toremove.end(), 
		[&toremovefrom](auto& queuedel) { queuedel.push_back('-'); toremovefrom.parsed.push_back(std::move(queuedel)); });
}

void clear(queues toclear, const std::string_view account)
{
	queue_list clearthis = open_queue(toclear, account);

	clearthis.parsed.clear();

	if (toclear == queues::post)
	{
		fs::remove_all(get_file_queue_directory(account));
	}
}

queue_list get(queues toget, const std::string_view account)
{
	return open_queue(toget, account);
}

std::vector<std::string> print(queues toprint, const std::string_view account)
{
	//prettyprint posts
	const queue_list printthis = open_queue(toprint, account);
	return std::vector<std::string> {printthis.parsed.begin(), printthis.parsed.end()};
}
