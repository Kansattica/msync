#include "queues.hpp"

#include <constants.hpp>
#include <filesystem.hpp>
#include <system_error>
#include <print_logger.hpp>
#include "../options/global_options.hpp"
#include "../postfile/outgoing_post.hpp"
#include <algorithm>
#include <array>
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

bool validate_file(const fs::path& attachpath)
{
	static constexpr std::array<std::string_view, 4> image_extensions = { ".png", ".jpg", ".jpeg", ".gif" };

	static constexpr std::array<std::string_view, 13> audiovisual_extensions =
	{ ".gifv", ".mp4", ".m4v", ".mov", ".webm", ".mp3", ".ogg", ".wav", ".flac", ".opus", ".aac", ".m4a", ".3gp" };

	constexpr auto eight_megabytes = 8 * 1024 * 1024;
	constexpr auto forty_megabytes = 40 * 1024 * 1024;

	//basically, if you don't do this .string() thing when printing a path, Windows wraps it in quotes and escapes the backslashes
	//I think .native() avoids an allocation, but that doesn't actually compile on MSVC when I tried
	if (!fs::is_regular_file(attachpath))
	{
		pl() << attachpath.string() << " is not a regular file. Skipping.\n";
		return false;
	}

	const auto extension = attachpath.extension();

	// this should really be a case insensitive comparison, but, uh, there's no real good portable way to do that with paths
	const auto compare_extension = [&extension](const auto& allowed_extension) { return extension == allowed_extension; };

	const auto file_size = fs::file_size(attachpath);

	bool is_image = std::any_of(image_extensions.begin(), image_extensions.end(), compare_extension), is_av = false;
	
	if (!is_image)
	{
		is_av = std::any_of(audiovisual_extensions.begin(), audiovisual_extensions.end(), compare_extension);
	}

	if (extension.empty())
	{
		pl() << "Warning: " << attachpath.string() << " doesn't seem to have an extension. Vanilla Mastodon might not know what to do with it.\n";
	}
	else if (is_image && file_size > eight_megabytes)
	{
		pl() << "Warning: " << attachpath.string() << " is an image over eight megabytes. Vanilla Mastodon may not accept it.\n";
	}
	else if (is_av && file_size > forty_megabytes)
	{
		pl() << "Warning: " << attachpath.string() << " is an audio or video file over forty megabytes. Vanilla Mastodon may not accept it.\n";
	}
	else if (!is_image && !is_av)
	{
		pl() << "Warning: Vanilla Mastodon might not support " << extension << " files as attachments.\n";
	}


	return true;
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

		if (!validate_file(attachpath))
			continue;

		attach = attachpath.string();

		err.clear();
	}
}

std::string queue_post(const fs::path& queuedir, const fs::path& postfile)
{
	fs::create_directories(queuedir);

	if (!fs::exists(postfile))
	{
		pl() << "Could not find " << postfile.string() << ". Skipping.\n";
		return {};
	}

	if (!fs::is_regular_file(postfile))
	{
		pl() << postfile.string() << " is not a file. Skipping.\n";
		return {};
	}

	fs::path copyto = queuedir / postfile.filename();

	if (fs::exists(copyto))
	{
		pl() << copyto.string() << " already exists. " << postfile.string() << " will be saved to ";
		unique_file_name(copyto);
		pl() << copyto.string() << '\n';
	}

	fs::copy(postfile, copyto);

	queue_attachments(copyto);

	return copyto.filename().string();
}

template <typename queue_t = queue_list>
queue_t open_queue(const queues to_open, const std::string_view account)
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

	return queue_t{ qfile / to_append };
}

void enqueue(const queues toenqueue, const std::string_view account, const std::vector<std::string>& add)
{
	queue_list toaddto = open_queue(toenqueue, account);

	if (toenqueue == queues::post)
	{
		const fs::path filequeuedir = get_file_queue_directory(account);
		std::transform(add.begin(), add.end(), std::back_inserter(toaddto.parsed), [&filequeuedir](const auto& id)
			{
				return queue_post(filequeuedir, id);
			});
	}
	else
	{
		// hm, this is (add.size() * toaddto.size()) string compares, which isn't great, performance-wise
		// but I think it's worth it to eliminate duplicates from the queue, since that saves network requests down the line.
		// I do it like this because std::unique only works on adjacent duplicates, and sorting the list would destroy the order
		// I might be able to get a speedup by copying toaddto into a std::unordered_set, but I'm not sure adding a bunch of
		// allocations to the mix really helps for the small input sizes msync deals with. 
		// similar to the dequeue situation

		// you could argue that order doesn't matter for favs and boosts, and I think that, too, but 
		// - it absolutely matters for posts, especially since posts can be replies to others
		// - if this part of the program is called 'queue', it should implement a queue
		// - if I had to write this again, I'd probably not have separate queues for boosts and favs, and just
		// have it be a big list of API calls to make. I'm probably going to revisit that decision once I start 
		// adding profile updates, poll voting, and other stuff that can get the "fire and forget" treatment like
		// favs and boosts do

		const auto does_not_contain = [&toaddto](const std::string& adding)
		{
			return std::find(toaddto.parsed.begin(), toaddto.parsed.end(), adding) == toaddto.parsed.end();
		};

		std::copy_if(add.begin(), add.end(), std::back_inserter(toaddto.parsed), does_not_contain);
	}

	// consider looking for those "delete" guys, the ones with the - at the end, and having this cancel them out, 
	// but "unboost and reboost", for example, is a valid thing to want to do.
}

void dequeue_post(const fs::path& queuedir, const fs::path& filename)
{
	if (!fs::remove(queuedir / filename))
	{
		pl() << "Could not delete " << filename << ", could not find it in " << queuedir.string() << '\n';
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

	// what we really want is the set difference between these two, but:
	// - it would require making a sorted or std::unordered_set copy of toremovefrom and same for toremove
	// - it'd still be an O(n) operation to remove existing entries from toremovefrom because toremovefrom has to stay in order
	// - you'd have have to do two set differences- one for the things to be removed from toremovefrom and one
	//   for the things in toremove that get enqueued with the minus sign

	// put the ones to keep first, and the ones to remove last
	const auto removefrom_pivot = std::stable_partition(toremovefrom.parsed.begin(), toremovefrom.parsed.end(),
		[&toremove](const auto& id) {
			return std::find(toremove.begin(), toremove.end(), id) == toremove.end();
		});


	// put the ones that'll be removed from the queue first and the rest after 

	// all the ones in toremovefrom that are also in toremove are at the end, so we only have to check those.
	// again, partitioning is O(n) and std::find is O(n), so this is another n^2 algorithm.
	// but these should be relatively small and so I highly doubt it's going to matter. If it does,
	// I could copy the things that'll be removed from the queue to an unordered set. Again, overkill for this.
	const auto toremove_pivot = std::stable_partition(toremove.begin(), toremove.end(),
		[&toremovefrom, removefrom_pivot](const auto& id) {
			return std::find(removefrom_pivot, toremovefrom.parsed.end(), id) != toremovefrom.parsed.end();
		});

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
	const readonly_queue_list printthis = open_queue<readonly_queue_list>(toprint, account);
	std::vector<std::string> toreturn(printthis.parsed.size());
	std::move(printthis.parsed.begin(), printthis.parsed.end(), toreturn.begin());
	return toreturn;
}
