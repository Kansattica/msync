#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/postlist/post_list.hpp"
#include "../lib/entities/entities.hpp"

#include <utility>
#include <array>
#include <filesystem.hpp>
#include <string_view>

constexpr std::string_view expected_content_nocw = R"(id: contentnocw
url: https://website.egg/contentnocw
author: regular@website.egg
body: This is a...
 test post.
visibility: public
posted on: 10:54AM 11-14-2019
0 favs | 1 boosts | 2 replies
--------------
)";

constexpr std::string_view expected_content_cw = R"(id: contentcw
url: https://website.egg/contentcw
author: afriend
cw: test inside
body: This is another test post.
visibility: unlisted
posted on: 10:55AM 11-14-2019
2 favs | 3 boosts | 4 replies
--------------
)";

constexpr std::string_view expected_justattachments = R"(id: justattachments
url: https://website.egg/justattachments
author: someone@online.egg [bot]
visibility: private
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
posted on: 10:56AM 11-14-2019
50 favs | 600 boosts | 7000 replies
--------------
)";

constexpr std::string_view expected_everything = R"(id: everything
url: https://website.egg/everything
author: cyberfriend [bot]
boosted by: meatbooster@different.website.egg
reply to: 123456
boost of: https://different.website.egg/goodpost
cw: Wow! A post!
body: Imagine: a post for you. :blobcat:
visibility: direct
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
with a newline
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
I am also a description!
posted on: 10:57AM 11-14-2019
8 favs | 99 boosts | 100000 replies
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
	content_nocw.author.account_name =  "regular@website.egg";
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
	everything.boosted_by_bot = false;
	everything.attachments = { {"https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488", "this is a description\nwith a newline"}, {"https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488", "I am also a description!"} };
	everything.favorites = 8;
	everything.boosts = 99;
	everything.replies = 100000;
	everything.author.account_name = "cyberfriend";
	everything.author.is_bot = true;
	return everything;
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

		const static std::array<status_test_case, 4> statuses
		{
			status_test_case{ content_nocw, expected_content_nocw },
			status_test_case{ content_cw, expected_content_cw },
			status_test_case{ justattachments, expected_justattachments },
			status_test_case{ everything, expected_everything }
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

constexpr std::string_view expected_fav = "at 10:50 AM 11/15/2019, localhuman favorited your post:\n";
constexpr std::string_view expected_boost = "at 10:51 AM 11/15/2019, localbot [bot] boosted your post:\n";
constexpr std::string_view expected_mention = "at 10:52 AM 11/15/2019, remotehuman@crime.egg mentioned you:\n";
constexpr std::string_view expected_follow = "at 10:53 AM 11/15/2019, remotebot@crime.egg [bot] followed you.";

struct notif_test_case
{
	const mastodon_notification& notif;
	const std::string_view expected_notif;
	const std::string_view expected_status;
};

mastodon_notification make_favorite()
{
	mastodon_notification notif;
	notif.account.account_name = "localhuman";
	notif.account.is_bot = false;
	notif.created_at = "10:50 AM 11/15/2019";
	notif.status = make_nocw();
	notif.type = notif_type::favorite;
	return notif;
}

mastodon_notification make_boost()
{
	mastodon_notification notif;
	notif.account.account_name = "localbot";
	notif.account.is_bot = true;
	notif.created_at = "10:51 AM 11/15/2019";
	notif.status = make_cw();
	notif.type = notif_type::boost;
	return notif;
}

mastodon_notification make_mention()
{
	mastodon_notification notif;
	notif.account.account_name = "remotehuman@crime.egg";
	notif.account.is_bot = false;
	notif.created_at = "10:52 AM 11/15/2019";
	notif.status = make_attachments();
	notif.type = notif_type::mention;
	return notif;
}

mastodon_notification make_follow()
{
	mastodon_notification notif;
	notif.account.account_name = "remotebot@crime.egg";
	notif.account.is_bot = true;
	notif.created_at = "10:53 AM 11/15/2019";
	notif.type = notif_type::follow;
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

		const static std::array<notif_test_case, 4> notifs
		{
			notif_test_case{ fav, expected_fav, expected_content_nocw },
			notif_test_case{ boost, expected_boost, expected_content_cw },
			notif_test_case{ mention, expected_mention, expected_justattachments },
			notif_test_case{ follow, expected_follow, "\n--------------\n"}
		};

		test_file fi{ "postlist.test" };
		WHEN("One of the notifications is serialized with post_list")
		{
			const auto& test_case = GENERATE(from_range(notifs));

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
			const auto& test_case = GENERATE(from_range(notifs));
			const auto& other_test_case = GENERATE(from_range(notifs));

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
			const auto& test_case = GENERATE(from_range(notifs));
			const auto& other_test_case = GENERATE(from_range(notifs));

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
