#include <catch2/catch.hpp>

#include <fstream>

#include <string>
#include <string_view>

#include <print_logger.hpp>

#include "test_helpers.hpp"
#include "../postfile/outgoing_post.hpp"

SCENARIO("outgoing_post correctly reads and writes posts.", "[long_run][problem]")
{
	logs_off = true;
	test_file fi{ "outfile" };
	GIVEN("An outgoing_post with only text is filled and destroyed")
	{
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
		const auto testtext = GENERATE(as<std::string_view>{},
			"this is a post!",
			"Rad post for you, baby.",
			"This\none's\tgot new\n\nlines",
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
		const auto testtext = GENERATE(as<std::string_view>{},
			"Rad post for you, baby.",
			"This\n\none's\tgot newlines",
			"phony=option",
			"#lookslikeacomment",
			" ---" );

		const bool snip = GENERATE(true, false);
		
		const auto content_warning = GENERATE(as<std::string_view>{},
			"", "that good good stuff", "=");

		const auto reply_to = GENERATE(as<std::string_view>{},
			"", "123980123", "X");

		const auto reply_id = GENERATE(as<std::string_view>{},
			"", "Hi!");

		const auto visibility = GENERATE(
			std::make_pair("", visibility::pub),
			std::make_pair("public", visibility::pub),
			std::make_pair("private", visibility::priv),
			std::make_pair("followersonly", visibility::priv),
			std::make_pair("unlisted", visibility::unlisted),
			std::make_pair("direct", visibility::direct),
			std::make_pair("dm", visibility::direct));

		//it would be really nice if these could be vectors of string views,
		//but it's way more convenient to be able to compare the vectors with ==
		//so here's hoping small string optimization helps here
		const auto attachments = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{"an attachment"},
			std::vector<std::string>{"hi", "there"},
			std::vector<std::string>{"four", "entire", "attachments", "foryou"}
			);

		auto descriptions = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{"describing: an attachment"},
			std::vector<std::string>{"describing: hi", "describing: there"},
			std::vector<std::string>{"describing: four", "", "describing: attachments", "describing: foryou"}
			);

		{
			std::ofstream of{ fi.filename };
			if (!content_warning.empty())
				of << "cw=" << content_warning << '\n';

			if (!reply_to.empty())
				of << "reply_to=" << reply_to << '\n';

			if (!reply_id.empty())
				of << "reply_id=" << reply_id << '\n';

			if (visibility.first[0] != '\0')
				of << "visibility=" << visibility.first << '\n';

			if (!attachments.empty())
			{
				of << "attach=";
				for (const auto& attach : attachments)
				{
					of << attach << ',';
				}
				of << '\n';
			}

			if (!descriptions.empty())
			{
				of << "descriptions=";
				for (const auto& describe : descriptions)
				{
					of << describe << ',';
				}
				of << '\n';
			}

			if (snip)
				of << "---\n";

			of << testtext;
		}


		WHEN("A new outgoing_post is made from the same file")
		{
			{
				outgoing_post result{ fi.filename };

				THEN("everything is as expected.")
				{
					REQUIRE(result.parsed.text == testtext);
					REQUIRE(result.parsed.vis == visibility.second);
					REQUIRE(result.parsed.attachments == attachments);

					REQUIRE(result.parsed.descriptions.size() <= result.parsed.attachments.size());

					if (attachments.size() < descriptions.size())
						descriptions.resize(attachments.size());

					REQUIRE(result.parsed.descriptions == descriptions);

					REQUIRE(result.parsed.content_warning == content_warning);
					REQUIRE(result.parsed.reply_to_id == reply_to);
					REQUIRE(result.parsed.reply_id == reply_id);
				}
			}

			AND_WHEN("That post is destroyed and a new one is created")
			{
				outgoing_post result{ fi.filename };

				THEN("everything is as expected.")
				{
					REQUIRE(result.parsed.text == testtext);
					REQUIRE(result.parsed.vis == visibility.second);
					REQUIRE(result.parsed.attachments == attachments);

					REQUIRE(result.parsed.descriptions.size() <= result.parsed.attachments.size());

					if (attachments.size() < descriptions.size())
						descriptions.resize(attachments.size());

					// if the last description is empty, that's okay, just remove it
					// an attachment with an empty description is the same as an attachment with no description
					// but we want to honor empty descriptions in the middle in case some attachments have descriptions and some don't.
					if (!descriptions.empty() && descriptions.back().empty())
						descriptions.pop_back();

					REQUIRE(result.parsed.descriptions == descriptions);

					REQUIRE(result.parsed.content_warning == content_warning);
					REQUIRE(result.parsed.reply_to_id == reply_to);
					REQUIRE(result.parsed.reply_id == reply_id);
				}
			}
		}

		WHEN("A new readonly_outgoing_post is made from the same file")
		{
			{
				readonly_outgoing_post result{ fi.filename };

				THEN("everything is as expected.")
				{
					REQUIRE(result.parsed.text == testtext);
					REQUIRE(result.parsed.vis == visibility.second);
					REQUIRE(result.parsed.attachments == attachments);

					REQUIRE(result.parsed.descriptions.size() <= result.parsed.attachments.size());

					if (attachments.size() < descriptions.size())
						descriptions.resize(attachments.size());

					REQUIRE(result.parsed.descriptions == descriptions);

					REQUIRE(result.parsed.content_warning == content_warning);
					REQUIRE(result.parsed.reply_to_id == reply_to);
					REQUIRE(result.parsed.reply_id == reply_id);
				}

				result.parsed.text = "some garbage";
				result.parsed.content_warning = "some more garbage";
				result.parsed.reply_to_id = "some jerk";
				result.parsed.attachments.push_back("bad attachment");
				result.parsed.descriptions.push_back("bad description");
			}

			AND_WHEN("That post is modified, destroyed and a new one is created")
			{
				readonly_outgoing_post result{ fi.filename };

				THEN("everything is as expected.")
				{
					REQUIRE(result.parsed.text == testtext);
					REQUIRE(result.parsed.vis == visibility.second);
					REQUIRE(result.parsed.attachments == attachments);

					REQUIRE(result.parsed.descriptions.size() <= result.parsed.attachments.size());

					if (attachments.size() < descriptions.size())
						descriptions.resize(attachments.size());

					REQUIRE(result.parsed.descriptions == descriptions);

					REQUIRE(result.parsed.content_warning == content_warning);
					REQUIRE(result.parsed.reply_to_id == reply_to);
					REQUIRE(result.parsed.reply_id == reply_id);

					// no backup file is made
					REQUIRE_FALSE(fs::exists(fi.filenamebak));
				}
			}
		}


	}
}

SCENARIO("outgoing_post is nothrow move constructible and assignable.")
{
	static_assert(std::is_nothrow_move_constructible<outgoing_post>::value, "outgoing_posts should be nothrow move constructible.");
	static_assert(std::is_nothrow_move_assignable<outgoing_post>::value, "outgoing_posts should be nothrow move assignable.");
	static_assert(std::is_copy_constructible<outgoing_post>::value == false, "outgoing_posts should not be copy constructible.");
	static_assert(std::is_copy_assignable<outgoing_post>::value == false, "outgoing_posts should not be copy assignable.");
}
