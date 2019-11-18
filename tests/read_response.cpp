#include <catch2/catch.hpp>

#include "../lib/sync/read_response.hpp"
#include "../lib/entities/entities.hpp"
#include "../util/util.hpp"

#include <utility>
#include <string_view>

bool operator==(const mastodon_account_field& lhs, const mastodon_account_field& rhs)
{
	return lhs.name == rhs.name && lhs.value == rhs.value;
}

SCENARIO("read_status correctly reads and cleans the relevant fields from a JSON status.")
{
	GIVEN("A json string representing a status with just content and no attachments.")
	{
		constexpr std::string_view status_json = R"({"id":"103144017685933985","created_at":"2019-11-15T21:20:09.004Z","in_reply_to_id":null,"in_reply_to_account_id":null,"sensitive":false,"spoiler_text":"","visibility":"public","language":"en","uri":"https://test.website.egg/users/BestGirlGrace/statuses/103144017685933985","url":"https://test.website.egg/@BestGirlGrace/103144017685933985","replies_count":2,"reblogs_count":3,"favourites_count":1,"content":"\u003cp\u003eUnit tests are well and good, but they\u0026apos;re no substitute for real-world testing. For example, until recently, msync would have a problem if an image description contained a comma and I didn\u0026apos;t realize that until I was composing that last post.\u003c/p\u003e","reblog":null,"application":{"name":"msync","website":"https://github.com/kansattica/msync"},"account":{"id":"1","username":"BestGirlGrace","acct":"BestGirlGrace","display_name":"Secret Government Grace :qvp:","locked":false,"bot":false,"created_at":"2018-08-16T04:45:49.523Z","note":"\u003cp\u003eThe buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\u003c/p\u003e\u003cp\u003eHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter\u003c/p\u003e","url":"https://test.website.egg/@BestGirlGrace","avatar":"https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299","avatar_static":"https://test.website.egg/system/accounts/avatars/000/000/001/static/2c3b6b7ff75a3d40.png?1573254299","header":"https://test.website.egg/system/accounts/headers/000/000/001/original/ba0b91a0c6545d9a.gif?1536301933","header_static":"https://test.website.egg/system/accounts/headers/000/000/001/static/ba0b91a0c6545d9a.png?1536301933","followers_count":1410,"following_count":702,"statuses_count":45051,"last_status_at":"2019-11-15T22:22:39.378Z","emojis":[{"shortcode":"qvp","url":"https://test.website.egg/system/custom_emojis/images/000/036/475/original/3470be8e5f2bf943.png?1564727510","static_url":"https://test.website.egg/system/custom_emojis/images/000/036/475/static/3470be8e5f2bf943.png?1564727510","visible_in_picker":true}],"fields":[{"name":"Pronouns","value":"she/her","verified_at":null},{"name":"Hornt Writing","value":"\u003ca href=\"https://perfect.hypnovir.us\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003eperfect.hypnovir.us\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":"2019-07-08T07:50:47.669+00:00"},{"name":"Fax Number","value":"(580) 4-GRACE-5","verified_at":null},{"name":"I made","value":"\u003ca href=\"https://github.com/Kansattica/Fluency\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003egithub.com/Kansattica/Fluency\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":null}]},"media_attachments":[],"mentions":[],"tags":[],"emojis":[],"card":null,"poll":null})";

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

				const std::vector<mastodon_account_field> expected_fields = {
					{ "Pronouns", "she/her" },
					{ "Hornt Writing", "https://perfect.hypnovir.us" },
					{ "Fax Number", "(580) 4-GRACE-5" },
					{ "I made", "https://github.com/Kansattica/Fluency" },
				};

				REQUIRE(status.author.id == "1");
				REQUIRE(status.author.account_name == "BestGirlGrace");
				REQUIRE(status.author.display_name == "Secret Government Grace :qvp:");
				REQUIRE(status.author.note == "The buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\nHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter");
				REQUIRE(status.author.url == "https://test.website.egg/@BestGirlGrace");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299");
				REQUIRE(status.author.fields == expected_fields);
				REQUIRE(status.author.is_bot == false);
			}
		}
	}

	GIVEN("A json string representing a boosted status with no body and an attachment.")
	{
		constexpr std::string_view status_json = R"({"id":"103139507032254843","created_at":"2019-11-15T02:13:01.819Z","in_reply_to_id":null,"in_reply_to_account_id":null,"sensitive":false,"spoiler_text":"","visibility":"public","language":null,"uri":"https://test.website.egg/users/BestGirlGrace/statuses/103139507032254843/activity","url":"https://test.website.egg/users/BestGirlGrace/statuses/103139507032254843/activity","replies_count":0,"reblogs_count":0,"favourites_count":0,"favourited":false,"reblogged":true,"muted":false,"content":"\u003cp\u003eHappy, Horny, Gay and Sassy\u003cbr\u003e\u003ca href=\"https://en.wikipedia.org/wiki/Happy%2C_Horny%2C_Gay_and_Sassy\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"ellipsis\"\u003een.wikipedia.org/wiki/Happy%2C\u003c/span\u003e\u003cspan class=\"invisible\"\u003e_Horny%2C_Gay_and_Sassy\u003c/span\u003e\u003c/a\u003e\u003c/p\u003e","reblog":{"id":"103139477977906929","created_at":"2019-11-15T02:05:36.000Z","in_reply_to_id":null,"in_reply_to_account_id":null,"sensitive":false,"spoiler_text":"","visibility":"public","language":"mg","uri":"https://botsin.space/users/tmnt/statuses/103139477824897157","url":"https://botsin.space/@tmnt/103139477824897157","replies_count":1,"reblogs_count":11,"favourites_count":0,"favourited":false,"reblogged":true,"muted":false,"content":"\u003cp\u003eHappy, Horny, Gay and Sassy\u003cbr\u003e\u003ca href=\"https://en.wikipedia.org/wiki/Happy%2C_Horny%2C_Gay_and_Sassy\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"ellipsis\"\u003een.wikipedia.org/wiki/Happy%2C\u003c/span\u003e\u003cspan class=\"invisible\"\u003e_Horny%2C_Gay_and_Sassy\u003c/span\u003e\u003c/a\u003e\u003c/p\u003e","reblog":null,"account":{"id":"51096","username":"tmnt","acct":"tmnt@botsin.space","display_name":"Wiki Titles Singable to TMNT","locked":false,"bot":true,"created_at":"2019-06-21T04:26:42.929Z","note":"\u003cp\u003eBot that posts Wiki titles that you can sing them to the TMNT song! See me also on Twitter: \u003ca href=\"https://twitter.com/wiki_tmnt\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003etwitter.com/wiki_tmnt\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e\u003c/p\u003e","url":"https://botsin.space/@tmnt","avatar":"https://test.website.egg/system/accounts/avatars/000/051/096/original/d6bcafe991182d18.jpeg?1561091201","avatar_static":"https://test.website.egg/system/accounts/avatars/000/051/096/original/d6bcafe991182d18.jpeg?1561091201","header":"https://test.website.egg/system/accounts/headers/000/051/096/original/4beff06bd41ec735.jpeg?1561091201","header_static":"https://test.website.egg/system/accounts/headers/000/051/096/original/4beff06bd41ec735.jpeg?1561091201","followers_count":135,"following_count":1,"statuses_count":3737,"last_status_at":"2019-11-16T00:02:43.207Z","emojis":[],"fields":[{"name":"Source","value":"\u003ca href=\"https://github.com/catleeball/tmnt_wikipedia_bot\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"ellipsis\"\u003egithub.com/catleeball/tmnt_wik\u003c/span\u003e\u003cspan class=\"invisible\"\u003eipedia_bot\u003c/span\u003e\u003c/a\u003e","verified_at":null},{"name":"Author","value":"\u003ca href=\"https://twitter.com/__eel__\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003etwitter.com/__eel__\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":null},{"name":"Logos","value":"\u003ca href=\"http://glench.com/tmnt/\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttp://\u003c/span\u003e\u003cspan class=\"\"\u003eglench.com/tmnt/\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":null},{"name":"Concept","value":"\u003ca href=\"https://xkcd.com/1412\" rel=\"nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003exkcd.com/1412\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":null}]},"media_attachments":[{"id":"666498","type":"image","url":"https://test.website.egg/system/media_attachments/files/000/666/498/original/a23c1652a24441b2.png?1573783538","preview_url":"https://test.website.egg/system/media_attachments/files/000/666/498/small/a23c1652a24441b2.png?1573783538","remote_url":"https://files.botsin.space/media_attachments/files/004/375/668/original/dd80bba436a28d4f.png","text_url":null,"meta":{"original":{"width":608,"height":320,"size":"608x320","aspect":1.9},"small":{"width":551,"height":290,"size":"551x290","aspect":1.9}},"description":null,"blurhash":"U$Qvm_WBx]ayi_j[tRj[_4j]M{j[XlWUROay"}],"mentions":[],"tags":[],"emojis":[],"card":{"url":"https://en.wikipedia.org/wiki/Happy,_Horny,_Gay_and_Sassy","title":"Happy, Horny, Gay and Sassy - Wikipedia","description":"","type":"link","author_name":"","author_url":"","provider_name":"","provider_url":"","html":"","width":280,"height":286,"image":"https://test.website.egg/system/preview_cards/images/000/120/806/original/3ba6861ff85e4044.jpeg?1573783562","embed_url":""},"poll":null},"application":null,"account":{"id":"1","username":"BestGirlGrace","acct":"BestGirlGrace","display_name":"Secret Government Grace :qvp:","locked":false,"bot":false,"created_at":"2018-08-16T04:45:49.523Z","note":"\u003cp\u003eThe buzz in your brain, the tingle behind your eyes, the good girl sneaking through your thoughts. Your favorite free-floating, reality-hacking, mind-tweaking, shitposting, horny, skunky, viral, infowitch.\u003c/p\u003e\u003cp\u003eHeader by @CorruptveSpirit@twitter, avi by @dogscribss@twitter\u003c/p\u003e","url":"https://test.website.egg/@BestGirlGrace","avatar":"https://test.website.egg/system/accounts/avatars/000/000/001/original/2c3b6b7ff75a3d40.gif?1573254299","avatar_static":"https://test.website.egg/system/accounts/avatars/000/000/001/static/2c3b6b7ff75a3d40.png?1573254299","header":"https://test.website.egg/system/accounts/headers/000/000/001/original/ba0b91a0c6545d9a.gif?1536301933","header_static":"https://test.website.egg/system/accounts/headers/000/000/001/static/ba0b91a0c6545d9a.png?1536301933","followers_count":1410,"following_count":702,"statuses_count":45057,"last_status_at":"2019-11-16T00:10:54.958Z","emojis":[{"shortcode":"qvp","url":"https://test.website.egg/system/custom_emojis/images/000/036/475/original/3470be8e5f2bf943.png?1564727510","static_url":"https://test.website.egg/system/custom_emojis/images/000/036/475/static/3470be8e5f2bf943.png?1564727510","visible_in_picker":true}],"fields":[{"name":"Pronouns","value":"she/her","verified_at":null},{"name":"Hornt Writing","value":"\u003ca href=\"https://perfect.hypnovir.us\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003eperfect.hypnovir.us\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":"2019-07-08T07:50:47.669+00:00"},{"name":"Fax Number","value":"(580) 4-GRACE-5","verified_at":null},{"name":"I made","value":"\u003ca href=\"https://github.com/Kansattica/Fluency\" rel=\"me nofollow noopener\" target=\"_blank\"\u003e\u003cspan class=\"invisible\"\u003ehttps://\u003c/span\u003e\u003cspan class=\"\"\u003egithub.com/Kansattica/Fluency\u003c/span\u003e\u003cspan class=\"invisible\"\u003e\u003c/span\u003e\u003c/a\u003e","verified_at":null}]},"media_attachments":[],"mentions":[],"tags":[],"emojis":[],"card":null,"poll":null})";

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
				REQUIRE(status.favorites == 0);
				REQUIRE(status.boosts == 11);
				REQUIRE(status.replies == 1);
				REQUIRE(status.attachments.size() == 1);
				REQUIRE(status.attachments[0].url == "https://test.website.egg/system/media_attachments/files/000/666/498/original/a23c1652a24441b2.png?1573783538");
				REQUIRE(status.attachments[0].description.empty());

				const std::vector<mastodon_account_field> expected_bot_fields = {
					{ "Source", "https://github.com/catleeball/tmnt_wikipedia_bot" },
					{ "Author", "https://twitter.com/__eel__" },
					{ "Logos", "http://glench.com/tmnt/" },
					{ "Concept", "https://xkcd.com/1412" },
				};

				REQUIRE(status.author.id == "51096");
				REQUIRE(status.author.account_name == "tmnt@botsin.space");
				REQUIRE(status.author.display_name == "Wiki Titles Singable to TMNT");
				REQUIRE(status.author.note == "Bot that posts Wiki titles that you can sing them to the TMNT song! See me also on Twitter: https://twitter.com/wiki_tmnt");
				REQUIRE(status.author.url == "https://botsin.space/@tmnt");
				REQUIRE(status.author.avatar == "https://test.website.egg/system/accounts/avatars/000/051/096/original/d6bcafe991182d18.jpeg?1561091201");
				REQUIRE(status.author.fields == expected_bot_fields);
				REQUIRE(status.author.is_bot == true);
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

