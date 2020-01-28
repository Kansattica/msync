#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/postlist/post_list.hpp"
#include "../lib/entities/entities.hpp"

#include <utility>
#include <array>
#include <filesystem.hpp>
#include <string_view>

constexpr std::string_view expected_content_nocw = R"(status id: contentnocw
url: https://website.egg/contentnocw
author: Normal Person (regular@website.egg)
body: This is a...
 test post.
visibility: public
posted on: 10:54AM 11-14-2019
0 favs | 1 boosts | 2 replies
--------------
)";

constexpr std::string_view expected_content_cw = R"(status id: contentcw
url: https://website.egg/contentcw
author: Alex Friendford (afriend)
cw: test inside
body: This is another test post.
visibility: unlisted
posted on: 10:55AM 11-14-2019
2 favs | 3 boosts | 4 replies
--------------
)";

constexpr std::string_view expected_justattachments = R"(status id: justattachments
url: https://website.egg/justattachments
author: Beepin' Online (someone@online.egg) [bot]
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
visibility: private
posted on: 10:56AM 11-14-2019
50 favs | 600 boosts | 7000 replies
--------------
)";

constexpr std::string_view expected_everything = R"(status id: everything
url: https://website.egg/everything
author: Cyberfriend: The Friendening (cyberfriend) [bot]
boosted by: Meat Boosterson (meatbooster@different.website.egg)
reply to: 123456
boost of: https://different.website.egg/goodpost
cw: Wow! A post!
body: Imagine: a post for you. :blobcat:
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
with a newline
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
I am also a description!
visibility: direct
posted on: 10:57AM 11-14-2019
8 favs | 99 boosts | 100000 replies
--------------
)";

constexpr std::string_view expected_expired_poll = R"(status id: gottapoll
url: https://website.egg/gottapoll
author: Normal Person (regular@website.egg)
body: what's the deal with airline food
poll id: isapoll
expired at: a time
 good things 4/10 votes (40%) [your vote]
 bad things 6/10 votes (60%)
visibility: public
posted on: 10:54AM 11-14-2019
0 favs | 1 boosts | 2 replies
--------------
)";

constexpr std::string_view expected_unexpired_poll = R"(status id: gottafuturepoll
url: https://website.egg/gottafuturepoll
author: Beepin' Online (someone@online.egg) [bot]
body: ladies???
poll id: isanotherpoll
expires at: a future time
 hello 3/10 votes (30%)
 sup 3/10 votes (30%)
 nougat 3/10 votes (30%)
 huh 1/10 votes (10%)
visibility: public
posted on: 10:54AM 11-14-2019
6 favs | 42 boosts | 9 replies
--------------
)";

struct status_test_case
{
	const mastodon_status& status;
	const std::string_view expected;
};


template <typename LHS, typename RHS>
size_t mismatch_loc(const LHS& lhs, const RHS& rhs)
{
	for (size_t i = 0; i < lhs.size(); i++)
	{
		if (i >= rhs.size()) { return i; }
		if (lhs[i] != rhs[i]) { return i; }
	}

	return lhs.size();
}

mastodon_status make_nocw()
{
	mastodon_status content_nocw;
	content_nocw.id = "contentnocw";
	content_nocw.url = "https://website.egg/contentnocw";
	content_nocw.content = "This is a...\n test post.";
	content_nocw.visibility = "public";
	content_nocw.created_at = "10:54AM 11-14-2019";
	content_nocw.favorites = 0;
	content_nocw.boosts = 1;
	content_nocw.replies = 2;
	content_nocw.author.account_name = "regular@website.egg";
	content_nocw.author.display_name = "Normal Person";
	content_nocw.author.is_bot = false;
	return content_nocw;
}

mastodon_status make_cw()
{
	mastodon_status content_cw;
	content_cw.id = "contentcw";
	content_cw.url = "https://website.egg/contentcw";
	content_cw.content_warning = "test inside";
	content_cw.content = "This is another test post.";
	content_cw.visibility = "unlisted";
	content_cw.created_at = "10:55AM 11-14-2019";
	content_cw.favorites = 2;
	content_cw.boosts = 3;
	content_cw.replies = 4;
	content_cw.author.account_name = "afriend";
	content_cw.author.display_name = "Alex Friendford";
	content_cw.author.is_bot = false;
	return content_cw;
}

mastodon_status make_attachments()
{
	mastodon_status justattachments;
	justattachments.id = "justattachments";
	justattachments.url = "https://website.egg/justattachments";
	justattachments.visibility = "private";
	justattachments.created_at = "10:56AM 11-14-2019";
	justattachments.attachments = { {"https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488", "this is a description"}, {"https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488", ""} };
	justattachments.favorites = 50;
	justattachments.boosts = 600;
	justattachments.replies = 7000;
	justattachments.author.account_name = "someone@online.egg";
	justattachments.author.display_name = "Beepin' Online";
	justattachments.author.is_bot = true;
	return justattachments;
}

mastodon_status make_everything()
{
	mastodon_status everything;
	everything.id = "everything";
	everything.url = "https://website.egg/everything";
	everything.content_warning = "Wow! A post!";
	everything.content = "Imagine: a post for you. :blobcat:";
	everything.visibility = "direct";
	everything.created_at = "10:57AM 11-14-2019";
	everything.reply_to_post_id = "123456";
	everything.original_post_url = "https://different.website.egg/goodpost";
	everything.boosted_by = "meatbooster@different.website.egg";
	everything.boosted_by_display_name = "Meat Boosterson";
	everything.boosted_by_bot = false;
	everything.attachments = { {"https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488", "this is a description\nwith a newline"}, {"https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488", "I am also a description!"} };
	everything.favorites = 8;
	everything.boosts = 99;
	everything.replies = 100000;
	everything.author.account_name = "cyberfriend";
	everything.author.display_name = "Cyberfriend: The Friendening";
	everything.author.is_bot = true;
	return everything;
}

mastodon_status make_expiredpoll()
{
	mastodon_status expiredpoll;
	expiredpoll.id = "gottapoll";
	expiredpoll.url = "https://website.egg/gottapoll";
	expiredpoll.content = "what's the deal with airline food";
	expiredpoll.visibility = "public";
	expiredpoll.created_at = "10:54AM 11-14-2019";
	expiredpoll.favorites = 0;
	expiredpoll.boosts = 1;
	expiredpoll.replies = 2;
	expiredpoll.author.account_name = "regular@website.egg";
	expiredpoll.author.display_name = "Normal Person";
	expiredpoll.author.is_bot = false;
	expiredpoll.poll = mastodon_poll{};
	expiredpoll.poll->id = "isapoll";
	expiredpoll.poll->expired = true;
	expiredpoll.poll->expires_at = "a time";
	expiredpoll.poll->total_votes = 10;
	expiredpoll.poll->voted_for = { 0 };
	expiredpoll.poll->you_voted = true;
	expiredpoll.poll->options = {
		{"good things", 4},
		{"bad things", 6},
	};

	return expiredpoll;
}

mastodon_status make_unexpiredpoll()
{
	mastodon_status unexpiredpoll;
	unexpiredpoll.id = "gottafuturepoll";
	unexpiredpoll.url = "https://website.egg/gottafuturepoll";
	unexpiredpoll.content = "ladies???";
	unexpiredpoll.visibility = "public";
	unexpiredpoll.created_at = "10:54AM 11-14-2019";
	unexpiredpoll.favorites = 6;
	unexpiredpoll.boosts = 42;
	unexpiredpoll.replies = 9;
	unexpiredpoll.author.account_name = "someone@online.egg";
	unexpiredpoll.author.display_name = "Beepin' Online";
	unexpiredpoll.author.is_bot = true;
	unexpiredpoll.poll = mastodon_poll{};
	unexpiredpoll.poll->id = "isanotherpoll";
	unexpiredpoll.poll->expired = false;
	unexpiredpoll.poll->expires_at = "a future time";
	unexpiredpoll.poll->total_votes = 10;
	unexpiredpoll.poll->voted_for = {};
	unexpiredpoll.poll->you_voted = false;
	unexpiredpoll.poll->options = {
		{"hello", 3},
		{"sup", 3},
		{"nougat", 3},
		{"huh", 1},
	};

	return unexpiredpoll;
}

size_t compare_window(std::string_view expected, std::string_view entire, size_t index)
{
	REQUIRE(expected == entire.substr(index, expected.size()));
	return index + expected.size();
}

SCENARIO("post_list correctly serializes lists of statuses.")
{
	GIVEN("Some mastodon_statuses and authors to serialize")
	{
		const static mastodon_status content_nocw = make_nocw();
		const static mastodon_status content_cw = make_cw();
		const static mastodon_status justattachments = make_attachments();
		const static mastodon_status everything = make_everything();
		const static mastodon_status expired_poll = make_expiredpoll();
		const static mastodon_status unexpired_poll = make_unexpiredpoll();

		const static std::array<status_test_case, 6> statuses
		{
			status_test_case{ content_nocw, expected_content_nocw },
			status_test_case{ content_cw, expected_content_cw },
			status_test_case{ justattachments, expected_justattachments },
			status_test_case{ everything, expected_everything },
			status_test_case{ expired_poll,  expected_expired_poll },
			status_test_case{ unexpired_poll, expected_unexpired_poll }
		};

		test_file fi{ "postlist.test" };
		WHEN("one status is written to a post_list and destroyed")
		{
			const auto& test_post = GENERATE_REF(from_range(statuses));

			{
				post_list<mastodon_status> list{ fi.filename };
				list.write(test_post.status);
			}

			THEN("the generated file is as expected.")
			{
				const auto actual = read_file(fi.filename);
				INFO("The strings mismatch at position " << mismatch_loc(actual, test_post.expected));
				REQUIRE(actual == test_post.expected);
			}
		}

		WHEN("two statuses are written to a post_list and destroyed")
		{
			const auto& test_post = GENERATE_REF(from_range(statuses));
			const auto& other_test_post = GENERATE_REF(from_range(statuses));

			{
				post_list<mastodon_status> list{ fi.filename };
				list.write(test_post.status);
				list.write(other_test_post.status);
			}

			THEN("the generated file is as expected.")
			{
				const auto actual = read_file(fi.filename);

				size_t idx = 0;
				idx = compare_window(test_post.expected, actual, idx);
				idx = compare_window(other_test_post.expected, actual, idx);
				REQUIRE(idx == actual.size());
			}
		}

		WHEN("two statuses are written to a post_list and destroyed one at a time.")
		{
			const auto& test_post = GENERATE_REF(from_range(statuses));
			const auto& other_test_post = GENERATE_REF(from_range(statuses));

			{
				post_list<mastodon_status> list{ fi.filename };
				list.write(test_post.status);
			}
			{
				post_list<mastodon_status> list{ fi.filename };
				list.write(other_test_post.status);
			}

			THEN("the generated file is as expected.")
			{
				const auto actual = read_file(fi.filename);

				size_t idx = 0;
				idx = compare_window(test_post.expected, actual, idx);
				idx = compare_window(other_test_post.expected, actual, idx);
				REQUIRE(idx == actual.size());
			}
		}
	}
}

constexpr std::string_view expected_fav = "notification id: 12345\nat 10:50 AM 11/15/2019, Alex Humansworth (localhuman) favorited your post:\n";
constexpr std::string_view expected_boost = "notification id: 67890\nat 10:51 AM 11/15/2019, Chad Beeps (localbot) [bot] boosted your post:\n";
constexpr std::string_view expected_mention = "notification id: 2567893344\nat 10:52 AM 11/15/2019, Egg Criminal (remotehuman@crime.egg) mentioned you:\n";
constexpr std::string_view expected_follow = "notification id: 9802347509287\nat 10:53 AM 11/15/2019, Electronic Egg Criminal (remotebot@crime.egg) [bot] followed you.";
constexpr std::string_view expected_poll = "notification id: 3412341\nat 10:53 AM 11/15/2019, Questionperson (quizboy@web.egg)'s poll ended:\n";

struct notif_test_case
{
	const mastodon_notification& notif;
	const std::string_view expected_notif;
	const std::string_view expected_status;
};

mastodon_notification make_favorite()
{
	mastodon_notification notif;
	notif.id = "12345";
	notif.account.account_name = "localhuman";
	notif.account.display_name = "Alex Humansworth";
	notif.account.is_bot = false;
	notif.created_at = "10:50 AM 11/15/2019";
	notif.status = make_nocw();
	notif.type = notif_type::favorite;
	return notif;
}

mastodon_notification make_boost()
{
	mastodon_notification notif;
	notif.id = "67890";
	notif.account.account_name = "localbot";
	notif.account.display_name = "Chad Beeps";
	notif.account.is_bot = true;
	notif.created_at = "10:51 AM 11/15/2019";
	notif.status = make_cw();
	notif.type = notif_type::boost;
	return notif;
}

mastodon_notification make_mention()
{
	mastodon_notification notif;
	notif.id = "2567893344";
	notif.account.account_name = "remotehuman@crime.egg";
	notif.account.display_name = "Egg Criminal";
	notif.account.is_bot = false;
	notif.created_at = "10:52 AM 11/15/2019";
	notif.status = make_attachments();
	notif.type = notif_type::mention;
	return notif;
}

mastodon_notification make_follow()
{
	mastodon_notification notif;
	notif.id = "9802347509287";
	notif.account.account_name = "remotebot@crime.egg";
	notif.account.display_name = "Electronic Egg Criminal";
	notif.account.is_bot = true;
	notif.created_at = "10:53 AM 11/15/2019";
	notif.type = notif_type::follow;
	return notif;
}

mastodon_notification make_poll()
{
	mastodon_notification notif;
	notif.id = "3412341";
	notif.account.account_name = "quizboy@web.egg";
	notif.account.display_name = "Questionperson";
	notif.account.is_bot = false;
	notif.created_at = "10:53 AM 11/15/2019";
	notif.status = make_expiredpoll();
	notif.type = notif_type::poll;
	return notif;
}

SCENARIO("post_list correctly serializes lists of notifications.")
{
	GIVEN("Some notifications to serialize with associated statuses.")
	{
		const static auto fav = make_favorite();
		const static auto boost = make_boost();
		const static auto mention = make_mention();
		const static auto follow = make_follow();
		const static auto poll = make_poll();

		const static std::array<notif_test_case, 5> notifs
		{
			notif_test_case{ fav, expected_fav, expected_content_nocw },
			notif_test_case{ boost, expected_boost, expected_content_cw },
			notif_test_case{ mention, expected_mention, expected_justattachments },
			notif_test_case{ follow, expected_follow, "\n--------------\n" },
			notif_test_case{ poll, expected_poll, expected_expired_poll },
		};

		test_file fi{ "postlist.test" };
		WHEN("One of the notifications is serialized with post_list")
		{
			const auto& test_case = GENERATE_REF(from_range(notifs));

			{
				post_list<mastodon_notification> list{ fi.filename };
				list.write(test_case.notif);
			}

			THEN("The single notification is written as expected.")
			{
				const std::string actual = read_file(fi.filename);

				size_t idx = 0;
				idx = compare_window(test_case.expected_notif, actual, idx);
				idx = compare_window(test_case.expected_status, actual, idx);
				REQUIRE(idx == actual.size());
			}
		}

		WHEN("Two notifications are serialized with post_list")
		{
			const auto& test_case = GENERATE_REF(from_range(notifs));
			const auto& other_test_case = GENERATE_REF(from_range(notifs));

			{
				post_list<mastodon_notification> list{ fi.filename };
				list.write(test_case.notif);
				list.write(other_test_case.notif);
			}

			THEN("Both notifications are written as expected.")
			{
				const std::string actual = read_file(fi.filename);

				size_t idx = 0;
				idx = compare_window(test_case.expected_notif, actual, idx);
				idx = compare_window(test_case.expected_status, actual, idx);
				idx = compare_window(other_test_case.expected_notif, actual, idx);
				idx = compare_window(other_test_case.expected_status, actual, idx);
				REQUIRE(idx == actual.size());
			}
		}

		WHEN("Two notifications are serialized with post_list destroyed between them")
		{
			const auto& test_case = GENERATE_REF(from_range(notifs));
			const auto& other_test_case = GENERATE_REF(from_range(notifs));

			{
				post_list<mastodon_notification> list{ fi.filename };
				list.write(test_case.notif);
			}
			{
				post_list<mastodon_notification> list{ fi.filename };
				list.write(other_test_case.notif);
			}

			THEN("Both notifications are written as expected.")
			{
				const std::string actual = read_file(fi.filename);

				size_t idx = 0;
				idx = compare_window(test_case.expected_notif, actual, idx);
				idx = compare_window(test_case.expected_status, actual, idx);
				idx = compare_window(other_test_case.expected_notif, actual, idx);
				idx = compare_window(other_test_case.expected_status, actual, idx);
				REQUIRE(idx == actual.size());
			}
		}
	}
}
