#include <catch2/catch.hpp>

#include <fstream>

#include "test_helpers.hpp"
#include "../postfile/outgoing_post.hpp"

SCENARIO("outgoing_post correctly reads and writes posts.")
{
	GIVEN("An outgoing_post with only text is filled and destroyed")
	{
		test_file fi{ "outfile" };

		{
			outgoing_post test{ fi.filename };
			test.parsed.text = "hey, I put some text in here\nand a newline";
		}

		WHEN("A new outgoing_post is made from the same file")
		{
			outgoing_post result{ fi.filename };

			THEN("the text is the same.")
			{
				REQUIRE(result.parsed.text == "hey, I put some text in here\nand a newline");
			}

			THEN("everything else is set to its default")
			{
				REQUIRE(result.parsed.vis == visibility::pub);
				REQUIRE(result.parsed.attachments.empty());
				REQUIRE(result.parsed.content_warning.empty());
				REQUIRE(result.parsed.reply_to_id.empty());
			}
		}
	}

	GIVEN("An outgoing_post with text and some attachments is filled and destroyed")
	{
		test_file fi{ "outfile" };

		{
			outgoing_post test{ fi.filename };
			test.parsed.text = "hey, buddy, wanna buy some...\n\ntext?";
			test.parsed.attachments = { "some file", "a different file", "hello!" };
		}

		WHEN("A new outgoing_post is made from the same file")
		{
			outgoing_post result{ fi.filename };

			THEN("the text is the same.")
			{
				REQUIRE(result.parsed.text == "hey, buddy, wanna buy some...\n\ntext?");
			}

			THEN("the attachments are parsed")
			{
				REQUIRE(result.parsed.attachments.size() == 3);
				REQUIRE(result.parsed.attachments == std::vector<std::string> {"some file", "a different file", "hello!"});
			}

			THEN("everything else is set to its default")
			{
				REQUIRE(result.parsed.vis == visibility::pub);
				REQUIRE(result.parsed.content_warning.empty());
				REQUIRE(result.parsed.reply_to_id.empty());
			}
		}
	}

	GIVEN("A raw text file.")
	{
		test_file fi{ "outfile" };

		auto testtext = GENERATE(as<std::string>{},
			"this is a post!",
			"Rad post for you, baby.",
			"This\none's\tgot newlines",
			"phony=option",
			"#lookslikeacomment",
			" ---" );

		{
			std::ofstream of{ fi.filename };
			of << testtext;
		}


		WHEN("A new outgoing_post is made from the same file")
		{
			outgoing_post result{ fi.filename };

			THEN("the text is the same.")
			{
				REQUIRE(result.parsed.text == testtext);
			}

			THEN("everything else is set to its default")
			{
				REQUIRE(result.parsed.vis == visibility::pub);
				REQUIRE(result.parsed.attachments.empty());
				REQUIRE(result.parsed.content_warning.empty());
				REQUIRE(result.parsed.reply_to_id.empty());
			}
		}
	}

	// hahaha this guy takes a while to run because of all the different combinations
	GIVEN("A cooked text file with and without the snip.")
	{
		test_file fi{ "outfile" };

		auto testtext = GENERATE(as<std::string>{},
			"Rad post for you, baby.",
			"This\n\none's\tgot newlines",
			"phony=option",
			"#lookslikeacomment",
			" ---" );

		bool snip = GENERATE(true, false);
		
		auto content_warning = GENERATE(as<std::string>{},
			"", "danger!", "that good good stuff", "=");

		auto reply_to = GENERATE(as<std::string>{},
			"", "123980123", "X");

		auto visibility = GENERATE(
			std::make_pair("", visibility::pub),
			std::make_pair("public", visibility::pub),
			std::make_pair("private", visibility::priv),
			std::make_pair("followersonly", visibility::priv),
			std::make_pair("unlisted", visibility::unlisted),
			std::make_pair("direct", visibility::direct),
			std::make_pair("dm", visibility::direct));

		auto attachments = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{"hi", "there"},
			std::vector<std::string>{"an attachment"}
			);

		{
			std::ofstream of{ fi.filename };
			if (!content_warning.empty())
				of << "cw=" << content_warning << '\n';

			if (!reply_to.empty())
				of << "reply_to=" << reply_to << '\n';

			if (visibility.first[0] != '\0')
				of << "visibility=" << visibility.first << '\n';

			if (!attachments.empty())
			{
				of << "attach=";
				for (auto& attach : attachments)
				{
					of << attach << ',';
				}
				of << '\n';
			}

			if (snip)
				of << "---\n";

			of << testtext;
		}


		WHEN("A new outgoing_post is made from the same file")
		{
			outgoing_post result{ fi.filename };

			THEN("everything is as expected.")
			{
				REQUIRE(result.parsed.text == testtext);
				REQUIRE(result.parsed.vis == visibility.second);
				REQUIRE(result.parsed.attachments == attachments);
				REQUIRE(result.parsed.content_warning == content_warning);
				REQUIRE(result.parsed.reply_to_id == reply_to);
			}
		}
	}
}
