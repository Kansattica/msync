#include "test_helpers.hpp"

#include <whereami.h>

#include <memory>
#include <random>
#include <fstream>
#include <iterator>
#include <array>
#include <algorithm>
#include <iostream>

#include "../lib/constants/constants.hpp"

fs::path _get_exe_location()
{
    // see https://github.com/gpakosz/whereami
    const size_t length = wai_getModulePath(nullptr, 0, nullptr);

	std::cout << "The module path is apparently " << length << " characters." << std::endl;

	auto path = std::string(length + 1, '\0');

    int dirname_length;
    wai_getExecutablePath(&path[0], length, &dirname_length);

	path.resize(dirname_length);
    return path;
}

const static fs::path _accountdir = _get_exe_location() / Account_Directory;
test_file account_directory()
{
	return test_file{ _accountdir };
}

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

void touch(const fs::path& totouch)
{
	std::ofstream of(totouch.c_str(), std::ios::out | std::ios::app);
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
