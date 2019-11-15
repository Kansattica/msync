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
	const static std::vector<mastodon_account_field> expected_fields = {
		{ "Pronouns", "she/her" },
		{ "Hornt Writing", "https://perfect.hypnovir.us" },
		{ "Fax Number", "(580) 4-GRACE-5" },
		{ "I made", "https://github.com/Kansattica/Fluency" },
	};

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
				REQUIRE(status.favorites == 1);
				REQUIRE(status.boosts == 3);
				REQUIRE(status.replies == 2);
				REQUIRE(status.attachments.empty());

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

