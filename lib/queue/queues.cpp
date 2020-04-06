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


fs::path get_file_queue_directory(const std::string& account)
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

	// the .string() turns Windows-style wide chars into regular chars. 
	// I don't know if it turns non-ASCII characters into nonsense or utf-8 or what, but
	// it doesn't matter here- if an extension has any of those, it's not gonna match.
	const auto extension = attachpath.extension().string();

	// this should really be a case insensitive comparison, but that can wait.
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
#if MSYNC_USE_BOOST
	boost::system::error_code err;
#else
	std::error_code err;
#endif
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


api_route get_route(const queues queue, const bool remove)
{
	switch (queue)
	{
	case queues::boost:
		return remove ? api_route::unboost : api_route::boost;
	case queues::fav:
		return remove ? api_route::unfav : api_route::fav;
	case queues::post:
		return remove ? api_route::unpost : api_route::post;
	}

	throw msync_exception("Whoops, that shouldn't happen in this get_route business.");
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

	if (copyto.extension() == ".bak")
	{
		plverb() << "Filenames ending in '.bak' can cause issues for msync. Removing it for msync's copy.\n";
		copyto.replace_extension();
	}

	if (fs::exists(copyto))
	{
		plverb() << copyto.string() << " already exists. " << postfile.string() << " will be saved to ";
		unique_file_name(copyto);
		plverb() << copyto.string() << '\n';
	}

	fs::copy(postfile, copyto);

	queue_attachments(copyto);

	return copyto.filename().string();
}

template <typename queue_t = queue_list>
queue_t open_queue(const std::string& account)
{
	fs::path qfile = options().account_directory_location / account;
	fs::create_directories(qfile);
	return queue_t{ qfile / Queue_Filename };
}

std::vector<api_call> to_api_calls(std::vector<std::string>&& add, api_route target_route)
{
	std::vector<api_call> to_return;
	to_return.reserve(add.size());
	std::transform(std::make_move_iterator(add.begin()), std::make_move_iterator(add.end()),
		std::back_inserter(to_return), [target_route](std::string&& id)
		{
			return api_call{ target_route, std::move(id) };
		});

	return to_return;
}

void enqueue(const queues toenqueue, const std::string& account, std::vector<std::string>&& add)
{
	queue_list toaddto = open_queue(account);
	const auto target_route = get_route(toenqueue, false);

	if (toenqueue == queues::post)
	{
		const fs::path filequeuedir = get_file_queue_directory(account);
		std::transform(add.begin(), add.end(), std::back_inserter(toaddto.parsed), [&filequeuedir, target_route](const auto& id)
			{
				return api_call{ target_route, queue_post(filequeuedir, id) };
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


		for (api_call& incoming_call : to_api_calls(std::move(add), target_route))
		{
			if (std::find(toaddto.parsed.begin(), toaddto.parsed.end(), incoming_call) == toaddto.parsed.end())
			{
				toaddto.parsed.push_back(std::move(incoming_call));
			}
		}
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

void dequeue(queues todequeue, const std::string& account, std::vector<std::string>&& remove)
{
	queue_list toremovefrom = open_queue(account);

	if (todequeue == queues::post)
	{
		// trim path names 
		std::transform(remove.begin(), remove.end(), remove.begin(), [](const auto& path) { return fs::path(path).filename().string(); });
	}

	auto toremove = to_api_calls(std::move(remove), get_route(todequeue, false));

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
			[&filequeuedir](const auto& apicall) { dequeue_post(filequeuedir, apicall.argument); });
	}

	toremovefrom.parsed.erase(removefrom_pivot, toremovefrom.parsed.end());

	//basically, if a thing isn't in the queue, enqueue removing that thing. unboosting, unfaving, deleting a post
	//consider removing duplicate removes?

	const auto remove_route = get_route(todequeue, true);
	std::for_each(toremove_pivot, toremove.end(),
		[&toremovefrom, remove_route](api_call& queuedel) { toremovefrom.parsed.push_back(api_call{ remove_route, std::move(queuedel.argument) }); });
}

void clear(queues toclear, const std::string& account)
{
	queue_list clearthis = open_queue(account);
	const auto toclearinsert = get_route(toclear, true);
	const auto toclearremove = get_route(toclear, false);

	clearthis.parsed.erase(std::remove_if(clearthis.parsed.begin(), clearthis.parsed.end(), [toclearinsert, toclearremove](const api_call& call)
		{
			return call.queued_call == toclearinsert || call.queued_call == toclearremove;
		}), clearthis.parsed.end());


	if (toclear == queues::post)
	{
		fs::remove_all(get_file_queue_directory(account));
	}
}

queue_list get(const std::string& account)
{
	return open_queue(account);
}

std::vector<std::string> print(const std::string& account)
{
	//prettyprint posts
	readonly_queue_list printthis = open_queue<readonly_queue_list>(account);
	std::vector<std::string> toreturn(printthis.parsed.size());
	std::transform(std::make_move_iterator(printthis.parsed.begin()), std::make_move_iterator(printthis.parsed.end()),
		toreturn.begin(), [](api_call&& call) 
		{
			const auto route_name = print_route(call.queued_call);
			std::string toreturn = std::move(call.argument);
			toreturn.insert(0, route_name.size() + 1, ' ');
			toreturn.replace(0, route_name.size(), route_name);
			return toreturn;
		});
	return toreturn;
}
