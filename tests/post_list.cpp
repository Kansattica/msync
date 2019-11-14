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
favs: 0 boosts: 1 replies: 2
--------------
)";

constexpr std::string_view expected_content_cw = R"(id: contentcw
url: https://website.egg/contentcw
author: afriend
cw: test inside
body: This is another test post.
visibility: unlisted
posted on: 10:55AM 11-14-2019
favs: 2 boosts: 3 replies: 4
--------------
)";

constexpr std::string_view expected_justattachments = R"(id: justattachments
url: https://website.egg/justattachments
author: someone@online.egg (bot)
visibility: private
attachments:
https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488
this is a description
https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488
posted on: 10:56AM 11-14-2019
favs: 50 boosts: 600 replies: 7000
--------------
)";

struct status_test_case
{
	mastodon_status& status;
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

SCENARIO("post_list correctly serializes lists of statuses.")
{
	GIVEN("Some mastodon_statuses and authors to serialize")
	{
		mastodon_account remoteauthor; remoteauthor.account_name = "regular@website.egg"; remoteauthor.is_bot = false;
		mastodon_account remotebotauthor; remotebotauthor.account_name = "someone@online.egg"; remotebotauthor.is_bot = true;
		mastodon_account localauthor; localauthor.account_name = "afriend"; localauthor.is_bot = false;
		mastodon_account localbotauthor; localbotauthor.account_name = "cyberfriend"; localbotauthor.is_bot = true;

		mastodon_status content_nocw;
		content_nocw.id = "contentnocw";
		content_nocw.url = "https://website.egg/contentnocw";
		content_nocw.content = "This is a...\n test post.";
		content_nocw.visibility = "public";
		content_nocw.created_at = "10:54AM 11-14-2019";
		content_nocw.favorites = 0;
		content_nocw.boosts = 1;
		content_nocw.replies = 2;
		content_nocw.author = remoteauthor;

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
		content_cw.author = localauthor;

		mastodon_status justattachments;
		justattachments.id = "justattachments";
		justattachments.url = "https://website.egg/justattachments";
		justattachments.visibility = "private";
		justattachments.created_at = "10:56AM 11-14-2019";
		justattachments.attachments = { {"https://fake.website.egg/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.png?1573544488", "this is a description"}, {"https://another.site/system/media_attachments/files/000/663/294/original/4536210b61b27ad2.mp3?1573544488", ""} };
		justattachments.favorites = 50;
		justattachments.boosts = 600;
		justattachments.replies = 7000;
		justattachments.author = remotebotauthor;

		std::array<status_test_case, 3> statuses 
		{ 
			status_test_case{ content_nocw, expected_content_nocw },
			status_test_case{ content_cw, expected_content_cw },
			status_test_case{ justattachments, expected_justattachments}
		};

		test_file fi{ "postlist.test" };
		WHEN("one status is added to a post_list and destroyed")
		{
			auto& test_case = GENERATE_REF(from_range(statuses));

			{
				post_list<mastodon_status> list{ fi.filename };
				list.toappend.push_back(std::move(test_case.status));
			}

			THEN("the generated file is as expected.")
			{
				const auto actual = read_file(fi.filename);
				INFO("The strings mismatch at position " << mismatch_loc(actual, test_case.expected));
				REQUIRE(actual == test_case.expected);
			}
		}
	}
}
