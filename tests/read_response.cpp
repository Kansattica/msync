#include <catch2/catch.hpp>

#include "../lib/sync/read_response.hpp"
#include "../lib/entities/entities.hpp"
#include "../util/util.hpp"

#include "read_response_json.hpp"

#include <utility>
#include <string_view>

bool operator==(const mastodon_account_field& lhs, const mastodon_account_field& rhs)
{
	return lhs.name == rhs.name && lhs.value == rhs.value;
}

bool operator==(const mastodon_poll_option& lhs, const mastodon_poll_option& rhs)
{
	return lhs.title == rhs.title && lhs.votes == rhs.votes;
}

const static std::vector<mastodon_account_field> expected_fields = {
	{ "Pronouns", "she/her" },
	{ "Hornt Writing", "https://perfect.hypnovir.us" },
	{ "Fax Number", "(580) 4-GRACE-5" },
	{ "I made", "https://github.com/Kansattica/Fluency" },
};

const static std::vector<mastodon_account_field> expected_new_fields = {
	{ "Pronouns", "she/her" },
	{ "Writing", "https://justa.normal.egg" },
	{ "Fax", "(580) 4-GRACE-5" },
	{ "Projects", "https://princess.software" },
};

const static std::vector<mastodon_account_field> expected_bot_fields = {
	{ "Source", "https://github.com/catleeball/tmnt_wikipedia_bot" },
	{ "Author", "https://twitter.com/__eel__" },
	{ "Logos", "http://glench.com/tmnt/" },
	{ "Concept", "https://xkcd.com/1412" },
};

SCENARIO("read_status correctly reads and cleans the relevant fields from a JSON status.")
{
	GIVEN("A json string representing a status with just content and no attachments.")
	{
		static constexpr std::string_view status_json = no_attach_status_json;

		WHEN("The string is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103144017685933985");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103144017685933985");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "Unit tests are well and good, but they're no substitute for real-world testing. For example, until recently, msync would have a problem if an image description contained a comma and I didn't realize that until I was composing that last post.");
				REQUIRE(status.visibility == "public");
				REQUIRE(status.created_at == "2019-11-15T21:20:09.004Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url.empty());
				REQUIRE(status.boosted_by.empty());
				REQUIRE(status.favorites == 1);
				REQUIRE(status.boosts == 3);
				REQUIRE(status.replies == 2);
				REQUIRE(status.attachments.empty());

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Secret Government Grace :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);

				REQUIRE_FALSE(status.poll.has_value());
			}
		}
	}

	GIVEN("A json string representing a boosted status with no body and an attachment.")
	{
		static constexpr std::string_view status_json = boosted_status_attachment_json;

		WHEN("The string is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103139507032254843");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103139507032254843/activity");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "Happy, Horny, Gay and Sassy\nhttps://en.wikipedia.org/wiki/Happy%2C_Horny%2C_Gay_and_Sassy");
				REQUIRE(status.visibility == "public");
				REQUIRE(status.created_at == "2019-11-15T02:05:36.000Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url ==	"https://botsin.space/users/tmnt/statuses/103139477824897157");
				REQUIRE(status.boosted_by == "BestGirlGrace");
				REQUIRE(status.boosted_by_bot == false);
				REQUIRE(status.boosted_by_display_name == "Secret Government Grace :qvp:");
				REQUIRE(status.favorites == 0);
				REQUIRE(status.boosts == 11);
				REQUIRE(status.replies == 1);
				REQUIRE(status.attachments.size() == 1);
				REQUIRE(status.attachments[0].url == "https://test.website.egg/system/media_attachments/files/000/666/498/original/a23c1652a24441b2.png?1573783538");
				REQUIRE(status.attachments[0].description.empty());

				REQUIRE(status.author.id == "51096");
				REQUIRE(status.author.account_name == "tmnt@botsin.space");
				REQUIRE(status.author.display_name == "Wiki Titles Singable to TMNT");
				REQUIRE(status.author.note == "Bot that posts Wiki titles that you can sing them to the TMNT song! See me also on Twitter: https://twitter.com/wiki_tmnt");
				REQUIRE(status.author.url == "https://botsin.space/@tmnt");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/051/096/original/d6bcafe991182d18.jpeg?1561091201");
				REQUIRE(status.author.fields == expected_bot_fields);
				REQUIRE(status.author.is_bot == true);

				REQUIRE_FALSE(status.poll.has_value());
			}
		}
	}

	GIVEN("A json string representing a status with an anonymous poll.")
	{
		static constexpr std::string_view status_json = anonymous_poll_json;

		WHEN("The status is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103563474507592337");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103563474507592337");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "test poll");
				REQUIRE(status.visibility == "public");
				REQUIRE(status.created_at == "2020-01-28T23:13:32.102Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url.empty());
				REQUIRE(status.boosted_by.empty());
				REQUIRE(status.favorites == 2);
				REQUIRE(status.boosts == 3);
				REQUIRE(status.replies == 4);
				REQUIRE(status.attachments.empty());

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Vx. Modemoiselle :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);

				REQUIRE(status.poll.has_value());
				REQUIRE(status.poll->expired);
				REQUIRE(status.poll->expires_at == "2020-01-29T01:13:32.093Z");
				REQUIRE(status.poll->id == "7786");
				REQUIRE(status.poll->options == std::vector<mastodon_poll_option> { {"yee", 13 }, { "haw", 20 } });
				REQUIRE(status.poll->total_votes == 33);
				REQUIRE(status.poll->voted_for.empty());
				REQUIRE_FALSE(status.poll->you_voted);
			}
		}
	}

	GIVEN("A json string representing a status with an anonymous poll with a null expiry.")
	{
		static constexpr std::string_view status_json = poll_null_expiry_json;

		WHEN("The status is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103563474507592337");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103563474507592337");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "test poll");
				REQUIRE(status.visibility == "public");
				REQUIRE(status.created_at == "2020-01-28T23:13:32.102Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url.empty());
				REQUIRE(status.boosted_by.empty());
				REQUIRE(status.favorites == 2);
				REQUIRE(status.boosts == 3);
				REQUIRE(status.replies == 4);
				REQUIRE(status.attachments.empty());

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Vx. Modemoiselle :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);

				REQUIRE(status.poll.has_value());
				REQUIRE_FALSE(status.poll->expired);
				REQUIRE(status.poll->expires_at == "");
				REQUIRE(status.poll->id == "7786");
				REQUIRE(status.poll->options == std::vector<mastodon_poll_option> { {"yee", 13 }, { "haw", 20 } });
				REQUIRE(status.poll->total_votes == 33);
				REQUIRE(status.poll->voted_for.empty());
				REQUIRE_FALSE(status.poll->you_voted);
			}
		}
	}

	GIVEN("A json string representing a status with a poll as seen by a logged-in user.")
	{
		static constexpr std::string_view status_json = logged_in_poll_json;

		WHEN("The status is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103563474507592337");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103563474507592337");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "test poll");
				REQUIRE(status.visibility == "public");
				REQUIRE(status.created_at == "2020-01-28T23:13:32.102Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url.empty());
				REQUIRE(status.boosted_by.empty());
				REQUIRE(status.favorites == 2);
				REQUIRE(status.boosts == 3);
				REQUIRE(status.replies == 4);
				REQUIRE(status.attachments.empty());

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Vx. Modemoiselle :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);

				REQUIRE(status.poll.has_value());
				REQUIRE_FALSE(status.poll->expired);
				REQUIRE(status.poll->expires_at == "2019-12-05T04:05:08.302Z");
				REQUIRE(status.poll->id == "34830");
				REQUIRE(status.poll->options == std::vector<mastodon_poll_option> { {"accept", 6 }, { "deny", 4 } });
				REQUIRE(status.poll->total_votes == 10);
				REQUIRE(status.poll->voted_for == std::vector<int> { 1 });
				REQUIRE(status.poll->you_voted);
			}
		}
	}

	GIVEN("A json string representing a status with some mentions.")
	{
		static constexpr std::string_view status_json = mentions_json;
		WHEN("The status is read.")
		{
			const auto status = read_status(status_json);

			THEN("The result is as expected.")
			{
				REQUIRE(status.id == "103642574110813163");
				REQUIRE(status.url == "https://test.website.egg/users/BestGirlGrace/statuses/103642574110813163");
				REQUIRE(status.content_warning.empty());
				REQUIRE(status.content == "@GoddessGrace@crime.egg @grace_ebooks this is a test, please ignore @tmnt@botsin.space @fakemention @wifelife");
				REQUIRE(status.visibility == "unlisted");
				REQUIRE(status.created_at == "2020-02-11T22:29:36.278Z");
				REQUIRE(status.reply_to_post_id.empty());
				REQUIRE(status.original_post_url.empty());
				REQUIRE(status.boosted_by.empty());
				REQUIRE(status.favorites == 0);
				REQUIRE(status.boosts == 0);
				REQUIRE(status.replies == 0);
				REQUIRE(status.attachments.empty());

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Vx. Modemoiselle :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);

				REQUIRE_FALSE(status.poll.has_value());
			}
		}
	}
}

SCENARIO("read_statuses correctly reads and cleans the relevant fields from an array of JSON statuses.")
{
	GIVEN("The two status json objects from the other test case, but in an array this time.")
	{
		static constexpr std::string_view statuses_json = statuses_array_json;

		WHEN("the string is parsed")
		{
			const auto statuses = read_statuses(statuses_json);

			THEN("The result is as expected.")
			{
				REQUIRE(statuses.size() == 2);

				REQUIRE(statuses[0].id == "103144017685933985");
				REQUIRE(statuses[0].url == "https://test.website.egg/users/BestGirlGrace/statuses/103144017685933985");
				REQUIRE(statuses[0].content_warning.empty());
				REQUIRE(statuses[0].content == "Unit tests are well and good, but they're no substitute for real-world testing. For example, until recently, msync would have a problem if an image description contained a comma and I didn't realize that until I was composing that last post.");
				REQUIRE(statuses[0].visibility == "public");
				REQUIRE(statuses[0].created_at == "2019-11-15T21:20:09.004Z");
				REQUIRE(statuses[0].reply_to_post_id.empty());
				REQUIRE(statuses[0].original_post_url.empty());
				REQUIRE(statuses[0].boosted_by.empty());
				REQUIRE(statuses[0].favorites == 1);
				REQUIRE(statuses[0].boosts == 3);
				REQUIRE(statuses[0].replies == 2);
				REQUIRE(statuses[0].attachments.empty());

				REQUIRE(statuses[0].author.id == "1");
				REQUIRE(statuses[0].author.account_name == "BestGirlGrace");
				REQUIRE(statuses[0].author.display_name == "Secret Government Grace :qvp:");
				REQUIRE(statuses[0].author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\n\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(statuses[0].author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(statuses[0].author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(statuses[0].author.fields == expected_fields);
				REQUIRE(statuses[0].author.is_bot == false);

				REQUIRE_FALSE(statuses[0].poll.has_value());

				REQUIRE(statuses[1].id == "103139507032254843");
				REQUIRE(statuses[1].url == "https://test.website.egg/users/BestGirlGrace/statuses/103139507032254843/activity");
				REQUIRE(statuses[1].content_warning.empty());
				REQUIRE(statuses[1].content == "Happy, Horny, Gay and Sassy\nhttps://en.wikipedia.org/wiki/Happy%2C_Horny%2C_Gay_and_Sassy");
				REQUIRE(statuses[1].visibility == "public");
				REQUIRE(statuses[1].created_at == "2019-11-15T02:05:36.000Z");
				REQUIRE(statuses[1].reply_to_post_id.empty());
				REQUIRE(statuses[1].original_post_url ==	"https://botsin.space/users/tmnt/statuses/103139477824897157");
				REQUIRE(statuses[1].boosted_by == "BestGirlGrace");
				REQUIRE(statuses[1].boosted_by_display_name == "Secret Government Grace :qvp:");
				REQUIRE(statuses[1].boosted_by_bot == false);
				REQUIRE(statuses[1].favorites == 0);
				REQUIRE(statuses[1].boosts == 11);
				REQUIRE(statuses[1].replies == 1);
				REQUIRE(statuses[1].attachments.size() == 1);
				REQUIRE(statuses[1].attachments[0].url == "https://test.website.egg/system/media_attachments/files/000/666/498/original/a23c1652a24441b2.png?1573783538");
				REQUIRE(statuses[1].attachments[0].description.empty());

				REQUIRE(statuses[1].author.id == "51096");
				REQUIRE(statuses[1].author.account_name == "tmnt@botsin.space");
				REQUIRE(statuses[1].author.display_name == "Wiki Titles Singable to TMNT");
				REQUIRE(statuses[1].author.note == "Bot that posts Wiki titles that you can sing them to the TMNT song! See me also on Twitter: https://twitter.com/wiki_tmnt");
				REQUIRE(statuses[1].author.url == "https://botsin.space/@tmnt");
				REQUIRE(statuses[1].author.avatar == "https://test.website.egg/system/accounts/avatars/000/051/096/original/d6bcafe991182d18.jpeg?1561091201");
				REQUIRE(statuses[1].author.fields == expected_bot_fields);
				REQUIRE(statuses[1].author.is_bot == true);

				REQUIRE_FALSE(statuses[1].poll.has_value());
			}
		}
	}

}

void assert_context_author(const mastodon_account& author)
{
	REQUIRE(author.id == "1");
	REQUIRE(author.account_name == "BestGirlGrace");
	REQUIRE(author.display_name == "Vx. Princess Grace :qvp:");
	REQUIRE(author.note == "I'm a gay crime skunk who writes internet porn and I demand to be treated with respect. Please don't follow me if you're under 18.\n\nFeel free to send a follow request if we've talked before. At least DM me or like a post so I know why you're here.\n\nHeader and avi by @fluxom_alt!");
	REQUIRE(author.url ==  "https://test.website.egg/@BestGirlGrace");
	REQUIRE(author.avatar == "https://anothertest.website.egg/system/accounts/avatars/000/000/001/original/8cb0e18d0db0f6c0.png");
	REQUIRE(author.fields == expected_new_fields);
	REQUIRE_FALSE(author.is_bot);

}

SCENARIO("read_context correctly deserializes a context object.")
{
	GIVEN("A json context object representing a thread.")
	{
		WHEN("That object is deserialized.")
		{
			const auto result = read_context(context_json);

			THEN("The results are as expected.")
			{
				REQUIRE(result.ancestors.size() == 1);
				REQUIRE(result.ancestors[0].id == "105539247080254892");
				REQUIRE(result.ancestors[0].url == "https://test.website.egg/users/BestGirlGrace/statuses/105539247080254892");
				REQUIRE(result.ancestors[0].content_warning.empty());
				REQUIRE(result.ancestors[0].content == "test thread post 1");
				REQUIRE(result.ancestors[0].visibility == "private");
				REQUIRE(result.ancestors[0].created_at == "2021-01-11T21:38:29.058Z");
				REQUIRE(result.ancestors[0].reply_to_post_id.empty());
				REQUIRE(result.ancestors[0].original_post_url.empty());
				REQUIRE(result.ancestors[0].boosted_by.empty());
				REQUIRE(result.ancestors[0].boosted_by_display_name.empty());
				REQUIRE_FALSE(result.ancestors[0].boosted_by_bot);
				REQUIRE(result.ancestors[0].favorites == 0);
				REQUIRE(result.ancestors[0].boosts == 0);
				REQUIRE(result.ancestors[0].replies == 0);
				REQUIRE(result.ancestors[0].attachments.empty());
				REQUIRE_FALSE(result.ancestors[0].poll.has_value());

				assert_context_author(result.ancestors[0].author);

				REQUIRE(result.descendants.size() == 2);
				REQUIRE(result.descendants[0].id == "105539248172091186");
				REQUIRE(result.descendants[0].url == "https://test.website.egg/users/BestGirlGrace/statuses/105539248172091186");
				REQUIRE(result.descendants[0].content_warning.empty());
				REQUIRE(result.descendants[0].content == "test thread post 3");
				REQUIRE(result.descendants[0].visibility == "private");
				REQUIRE(result.descendants[0].created_at == "2021-01-11T21:38:45.694Z");
				REQUIRE(result.descendants[0].reply_to_post_id == "105539247642691774");
				REQUIRE(result.descendants[0].original_post_url.empty());
				REQUIRE(result.descendants[0].boosted_by.empty());
				REQUIRE(result.descendants[0].boosted_by_display_name.empty());
				REQUIRE_FALSE(result.descendants[0].boosted_by_bot);
				REQUIRE(result.descendants[0].favorites == 0);
				REQUIRE(result.descendants[0].boosts == 0);
				REQUIRE(result.descendants[0].replies == 0);
				REQUIRE(result.descendants[0].attachments.empty());
				REQUIRE_FALSE(result.descendants[0].poll.has_value());

				assert_context_author(result.descendants[0].author);

				REQUIRE(result.descendants[1].id == "105539249606393432");
				REQUIRE(result.descendants[1].url == "https://test.website.egg/users/BestGirlGrace/statuses/105539249606393432");
				REQUIRE(result.descendants[1].content_warning == "you guessed it");
				REQUIRE(result.descendants[1].content == "test thread post four");
				REQUIRE(result.descendants[1].visibility == "private");
				REQUIRE(result.descendants[1].created_at == "2021-01-11T21:39:07.572Z");
				REQUIRE(result.descendants[1].reply_to_post_id == "105539248172091186");
				REQUIRE(result.descendants[1].original_post_url.empty());
				REQUIRE(result.descendants[1].boosted_by.empty());
				REQUIRE(result.descendants[1].boosted_by_display_name.empty());
				REQUIRE_FALSE(result.descendants[1].boosted_by_bot);
				REQUIRE(result.descendants[1].favorites == 0);
				REQUIRE(result.descendants[1].boosts == 0);
				REQUIRE(result.descendants[1].replies == 0);
				REQUIRE(result.descendants[1].attachments.empty());
				REQUIRE_FALSE(result.descendants[1].poll.has_value());

				assert_context_author(result.descendants[1].author);
			}
		}
	}
}

SCENARIO("read_upload_id correctly reads the ID from a JSON status.")
{
	GIVEN("A json string with the relevant fields.")
	{
		const auto test = GENERATE(
			std::make_pair(R"({"id":"hello", "uri":"https://website.egg/cool"})",
				"hello"),
			std::make_pair(R"({"id":"123456789", "uri":"https://website.egg/cool"})",
				"123456789"),
			std::make_pair(R"({"id":"1221"})",
				"1221"),
			std::make_pair(R"({"id":"22222", "uri":"https://website.egg/cool"})",
				"22222")
		);

		WHEN("The string is read.")
		{
			const auto result = read_upload_id(std::get<0>(test));

			THEN("The result is as expected.")
			{
				REQUIRE(std::get<1>(test) == result);
			}
		}
	}
}

