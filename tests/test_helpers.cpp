#include "test_helpers.hpp"

#include <random>
#include <fstream>
#include <iterator>
#include <array>
#include <algorithm>

#include "../lib/constants/constants.hpp"

#include "to_chars_patch.hpp"

std::vector<std::string> read_lines(const fs::path& toread)
{
	// all these .c_str()s are to make Boost happy.
	std::ifstream fin(toread.c_str());
	std::vector<std::string> toreturn;

	for (std::string line; std::getline(fin, line);)
	{
		toreturn.push_back(std::move(line));
	}

	return toreturn;
}

size_t count_files_in_directory(const fs::path& tocheck)
{
	if (!fs::is_directory(tocheck)) { return -1; }

	size_t count = 0;
	auto dir = fs::directory_iterator(tocheck);
	while (dir != fs::end(dir))
	{
		++count;
		++dir;
	}
	return count;
}

std::string read_file(const fs::path& file)
{
	std::ifstream fi(file.c_str(), std::ios::ate | std::ios::in);

	std::string content;
	content.reserve(fi.tellg());
	fi.seekg(0, std::ios::beg);
	return content.append(std::istreambuf_iterator(fi), std::istreambuf_iterator<char>());
}

std::minstd_rand gen(std::random_device{}());

bool flip_coin()
{
	static std::uniform_int_distribution<> dis(0, 1);
	return dis(gen) == 0;
}

int zero_to_n(int n)
{
	std::uniform_int_distribution<> dis(0, n);
	return dis(gen);
}

const fs::path& test_base_dir()
{
	const static test_dir base_dir = fs::temp_directory_path() / "msync_test_files";
	return base_dir.dirname;
}

// the .concat(to_string(random number)) guys exist so that we can use ctest to run tests in parallel.
// All you have to do is ensure that no two processes are trying to use the same filenames.
// the correct way to do this would probably be to get the process ID, but I'd have to do a bunch of platform-specific code for that.
test_file temporary_file()
{
	static std::array<char, 10> buffer;
	const static fs::path tempdir = (test_base_dir() / "msync_test_file_").concat(std::to_string(gen()));
	static unsigned int filecount = 0;

	std::string_view printed = sv_to_chars(filecount++, buffer);

	return test_file{ fs::path {tempdir}.concat(printed.begin(), printed.end()) };
}

test_dir temporary_directory()
{
	const static fs::path tempdir = (test_base_dir() / "msync_test_dir_").concat(std::to_string(gen()));;
	static unsigned int dircount = 0;
	static std::array<char, 10> buffer;

	std::string_view printed = sv_to_chars(dircount++, buffer);

	return test_dir{ fs::path {tempdir}.concat(printed.begin(), printed.end()) };
}

void make_status_json(std::string_view id, std::string& to_append)
{
	to_append += R"({"id": ")";
	to_append += id;
	to_append += R"(", "created_at" : "2019-06-18T05:25:54.100Z", "in_reply_to_id" : null, "in_reply_to_account_id" : null, "sensitive" : false, "spoiler_text" : "", "visibility" : "public", "language" : "en", "uri" : "https://test.website.egg/users/BestGirlGrace/statuses/102290918869507417", "url" : "https://test.website.egg/@BestGirlGrace/102290918869507417", "replies_count" : 8, "reblogs_count" : 24, "favourites_count" : 56, "content" : "\u003cp\u003eI think this has been done before BUT a world where the superhero/villain scene is a kink thing. Traditionally, the hero is the top and \u0026quot;wins\u0026quot;, though \u0026quot;gritty\u0026quot; pairs with villain tops are more common these days.\u003c/p\u003e", "reblog" : null, "application" : null, "account" : {"id":"1", "username" : "BestGirlGrace", "acct" : "BestGirlGrace", "display_name" : "Secret Government Grace :qvp:", "locked" : false, "bot" : false, "created_at" : "2018-08-16T04:45:49.523Z", "note" : "\u003cp\u003eThe buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\u003c/p\u003e\u003cp\u003eHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter\u003c/p\u003e", "url" : "https://test.website.egg/@BestGirlGrace", "avatar" : "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299", "avatar_static" : "https://test.website.egg/system/accounts/avatars/000/000/001/static/2c3b6b7ff75a3d40.png?1573254299", "header" : "https://test.website.egg/system/accounts/headers/000/000/001/original/ba0b91a0c6545d9a.gif?1536301933", "header_static" : "https://test.website.egg/system/accounts/headers/000/000/001/static/ba0b91a0c6545d9a.png?1536301933", "followers_count" : 1410, "following_count" : 702, "statuses_count" : 45048, "last_status_at" : "2019-11-15T21:23:54.043Z", "emojis" : [{"shortcode":"qvp", "url" : "https://test.website.egg/system/custom_emojis/images/000/036/475/original/3470be8e5f2bf943.png?1564727510", "static_url" : "https://test.website.egg/system/custom_emojis/images/000/036/475/static/3470be8e5f2bf943.png?1564727510", "visible_in_picker" : true}] , "fields" : [{"name":"Pronouns", "value" : "she/her", "verified_at" : null}, { "name":"Hornt Writing","value" : "\u003ca href=\"https://perfect.hypnovir.us\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003eperfect.hypnovir.us\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at" : "2019-07-08T07:50:47.669+00:00" }, { "name":"Fax Number","value" : "(580) 4-GRACE-5","verified_at" : null }, { "name":"I made","value" : "\u003ca href=\"https://github.com/Kansattica/Fluency\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003egithub.com/Kansattica/Fluency\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at" : null }] }, "media_attachments" : [] , "mentions" : [] , "tags" : [] , "emojis" : [] , "card" : null, "poll" : null})";
}

void make_notification_json(std::string_view id, std::string& to_append)
{
	static constexpr std::array<std::string_view, 5> notification_types{ "follow", "mention", "reblog", "favourite", "poll" };

	to_append += R"({"id": ")";
	to_append += id;
	to_append += R"(", "type" : ")";
	to_append += notification_types[zero_to_n(static_cast<int>(notification_types.size()) - 1)];
	to_append += R"(", "created_at" : "2020-07-19T06:26:55.101Z", "account" : {"id":"1", "username" : "BestGirlGrace", "acct" : "BestGirlGrace", "display_name" : "Secret Government Grace :qvp:", "locked" : false, "bot" : false, "created_at" : "2018-08-16T04:45:49.523Z", "note" : "\u003cp\u003eThe buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\u003c/p\u003e\u003cp\u003eHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter\u003c/p\u003e", "url" : "https://test.website.egg/@BestGirlGrace", "avatar" : "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299", "avatar_static" : "https://test.website.egg/system/accounts/avatars/000/000/001/static/2c3b6b7ff75a3d40.png?1573254299", "header" : "https://test.website.egg/system/accounts/headers/000/000/001/original/ba0b91a0c6545d9a.gif?1536301933", "header_static" : "https://test.website.egg/system/accounts/headers/000/000/001/static/ba0b91a0c6545d9a.png?1536301933", "followers_count" : 1410, "following_count" : 702, "statuses_count" : 45048, "last_status_at" : "2019-11-15T21:23:54.043Z", "emojis" : [{"shortcode":"qvp", "url" : "https://test.website.egg/system/custom_emojis/images/000/036/475/original/3470be8e5f2bf943.png?1564727510", "static_url" : "https://test.website.egg/system/custom_emojis/images/000/036/475/static/3470be8e5f2bf943.png?1564727510", "visible_in_picker" : true}] , "fields" : [{"name":"Pronouns", "value" : "she/her", "verified_at" : null}, { "name":"Hornt Writing","value" : "\u003ca href=\"https://perfect.hypnovir.us\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003eperfect.hypnovir.us\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at" : "2019-07-08T07:50:47.669+00:00" }, { "name":"Fax Number","value" : "(580) 4-GRACE-5","verified_at" : null }, { "name":"I made","value" : "\u003ca href=\"https://github.com/Kansattica/Fluency\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003egithub.com/Kansattica/Fluency\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at" : null }] },)";
	to_append += R"("status" : )";

	if (flip_coin())
	{
		make_status_json(id, to_append);
	}
	else
	{
		to_append += "null";
	}

	to_append += '}';
}

std::vector<std::string> make_expected_ids(const std::vector<std::string>& ids, std::string_view prefix)
{
	std::vector<std::string> toreturn(ids.size());
	std::transform(ids.begin(), ids.end(), toreturn.begin(), [&prefix](const std::string& id) 
		{
			return std::string{ id }.insert(0, prefix);
		});
	return toreturn;
}
