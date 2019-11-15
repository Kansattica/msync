#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../lib/postlist/post_list.hpp"
#include "../lib/entities/entities.hpp"

#include <utility>
#include <array>
#include <filesystem.hpp>
#include <string_view>

// these are std::strings because catch's StartsWith doesn't work with string_views
const static std::string expected_content_nocw = R"(id: contentnocw
url: https://website.egg/contentnocw
author: regular@website.egg
body: This is a...
 test post.
visibility: public
posted on: 10:54AM 11-14-2019
0 favs | 1 boosts | 2 replies
--------------
)";

const static std::string expected_content_cw = R"(id: contentcw
url: https://website.egg/contentcw
author: afriend
cw: test inside
body: This is another test post.
visibility: unlisted
posted on: 10:55AM 11-14-2019
2 favs | 3 boosts | 4 replies
--------------
)";

const static std::string expected_justattachments = R"(id: justattachments
url: https://website.egg/justattachments
author: someone@online.egg (bot)
visibility: private
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
posted on: 10:56AM 11-14-2019
50 favs | 600 boosts | 7000 replies
--------------
)";

const static std::string expected_everything = R"(id: everything
url: https://website.egg/everything
author: cyberfriend (bot)
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
	const std::string& expected;
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
	everything.attachments = { {"https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488", "this is a description\nwith a newline"}, {"https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488", "I am also a description!"} };
	everything.favorites = 8;
	everything.boosts = 99;
	everything.replies = 100000;
	everything.author.account_name = "cyberfriend";
	everything.author.is_bot = true;
	return everything;
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
				using namespace Catch::Matchers;
				const auto actual = read_file(fi.filename);
				REQUIRE_THAT(actual, StartsWith(test_post.expected) && EndsWith(other_test_post.expected));
				REQUIRE(actual.size() == test_post.expected.size() + other_test_post.expected.size());
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
				using namespace Catch::Matchers;
				const auto actual = read_file(fi.filename);
				REQUIRE_THAT(actual, StartsWith(test_post.expected) && EndsWith(other_test_post.expected));
				REQUIRE(actual.size() == test_post.expected.size() + other_test_post.expected.size());
			}
		}
	}
}
