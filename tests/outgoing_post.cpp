#include <catch2/catch.hpp>

#include <fstream>

#include <string>
#include <string_view>

#include <print_logger.hpp>

#include "test_helpers.hpp"
#include "../postfile/outgoing_post.hpp"

#include "../lib/fixlocale/fix_locale.hpp"

SCENARIO("outgoing_post correctly reads and writes posts.")
{
	logs_off = true;
	const test_file fi = temporary_file();
	GIVEN("An outgoing_post with only text is filled and destroyed")
	{
		{
			outgoing_post test{ fi.filename() };
			test.parsed.text = "hey, I put some text in here\nand a newline";
		}

		WHEN("A new outgoing_post is made from the same file")
		{
			const outgoing_post result{ fi.filename() };

			THEN("the text is the same.")
			{
				REQUIRE(result.parsed.text == "hey, I put some text in here\nand a newline");
			}

			THEN("everything else is set to its default")
			{
				REQUIRE(result.parsed.vis == visibility::default_vis);
				REQUIRE(result.parsed.attachments.empty());
				REQUIRE(result.parsed.content_warning.empty());
				REQUIRE(result.parsed.reply_to_id.empty());
			}
		}
	}

	GIVEN("An outgoing_post with text and some attachments is filled and destroyed")
	{
		{
			outgoing_post test{ fi.filename() };
			test.parsed.text = "hey, buddy, wanna buy some...\n\ntext?";
			test.parsed.attachments = { "some file", "a different file", "hello!" };
		}

		WHEN("A new outgoing_post is made from the same file")
		{
			const outgoing_post result{ fi.filename() };

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
				REQUIRE(result.parsed.vis == visibility::default_vis);
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
			std::ofstream of{ fi };
			of << testtext;
		}


		WHEN("A new outgoing_post is made from the same file")
		{
			const outgoing_post result{ fi.filename() };

			THEN("the text is the same.")
			{
				REQUIRE(result.parsed.text == testtext);
			}

			THEN("everything else is set to its default")
			{
				REQUIRE(result.parsed.vis == visibility::default_vis);
				REQUIRE(result.parsed.attachments.empty());
				REQUIRE(result.parsed.content_warning.empty());
				REQUIRE(result.parsed.reply_to_id.empty());
			}
		}
	}
}

void make_file(const fs::path& target_path, std::string_view content_warning, std::string_view reply_to, std::string_view reply_id, const char* visibility,
	const std::vector<std::string>& attachments, const std::vector<std::string>& descriptions, std::string_view testtext, bool snip)
{
	std::ofstream of{ target_path.c_str() }; // make boost happy
	if (!content_warning.empty())
		of << "cw=" << content_warning << '\n';

	if (!reply_to.empty())
		of << "reply_to=" << reply_to << '\n';

	if (!reply_id.empty())
		of << "reply_id=" << reply_id << '\n';

	if (visibility[0] != '\0')
		of << "visibility=" << visibility << '\n';


	for (const auto& attach : attachments)
	{
		of << "attach=" << attach << '\n';
	}

	for (const auto& describe : descriptions)
	{
		of << "description=" << describe << '\n';
	}

	if (snip)
		of << "---\n";

	of << testtext;
}

SCENARIO("outgoing_post can handle a bunch of combinations of cooked post with the snip.", "[long_run][long_run_outgoingpost]")
{
	logs_off = true;
	const test_file fi = temporary_file();
	GIVEN("A cooked text file with the snip.")
	{
		const auto testtext = GENERATE(as<std::string_view>{},
			"Rad post for you, baby.",
			"This\n\none's\tgot newlines",
			"phony=option",
			"#lookslikeacomment",
			" ---");

		const auto content_warning = GENERATE(as<std::string_view>{},
			"", "that good good stuff", "=");

		const auto reply_to = GENERATE(as<std::string_view>{},
			"", "123980123", "X");

		const auto reply_id = GENERATE(as<std::string_view>{},
			"", "Hi!");

		const auto visibility = GENERATE(
			std::make_pair("", visibility::default_vis),
			std::make_pair("default", visibility::default_vis),
			std::make_pair("public", visibility::pub),
			std::make_pair("private", visibility::priv),
			std::make_pair("followersonly", visibility::priv),
			std::make_pair("unlisted", visibility::unlisted),
			std::make_pair("direct", visibility::direct),
			std::make_pair("dm", visibility::direct));

		//it would be really nice if these could be vectors of string views,
		//but it's way more convenient to be able to compare the vectors with ==
		//so I'm hoping small string optimization helps here
		const auto attachments = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{"an attachment"},
			std::vector<std::string>{"h,i", "there"},
			std::vector<std::string>{"four", "entire", "attachments", "foryou"}
		);

		auto descriptions = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{"d: an attachment"},
			std::vector<std::string>{"d: h,i", "d: there"},
			std::vector<std::string>{"d: four", "", "d: attachments", "d: foryou"}
		);

		make_file(fi.filename(), content_warning, reply_to, reply_id, visibility.first, attachments, descriptions, testtext, true);

		WHEN("A new outgoing_post is made from the same file")
		{
			{
				const outgoing_post result{ fi.filename() };

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
				const outgoing_post result{ fi.filename() };

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
		}

		WHEN("A new readonly_outgoing_post is made from the same file")
		{
			{
				readonly_outgoing_post result{ fi.filename() };

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
				result.parsed.content_warning = "more garbage";
				result.parsed.reply_to_id = "some jerk";
				result.parsed.attachments.push_back("bad attachment");
				result.parsed.descriptions.push_back("bad description");
			}

			AND_WHEN("That post is modified, destroyed and a new one is created")
			{
				const readonly_outgoing_post result{ fi.filename() };

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
					REQUIRE_FALSE(fs::exists(fi.filenamebak()));
				}
			}
		}
	}
}

SCENARIO("outgoing_post can handle a bunch of combinations of cooked post without the snip.", "[long_run][long_run_outgoingpost][locale]")
{
	logs_off = true;
	fix_locale();
	const test_file fi = temporary_file();
	GIVEN("A cooked text file without the snip.")
	{
		const auto testtext = GENERATE(as<std::string_view>{},
			u8"Rad post for you, baby 🕶.",
			u8"This\n\none's\tgot newlines",
			u8"phony=option",
			u8"#lookslikeacomment",
			u8" ---");

		const auto content_warning = GENERATE(as<std::string_view>{},
			"", u8"that good good 🦨 stuff", "=");

		const auto reply_to = GENERATE(as<std::string_view>{},
			"", "123980123", "X");

		const auto reply_id = GENERATE(as<std::string_view>{},
			"", "Hi!");

		const auto visibility = GENERATE(
			std::make_pair("", visibility::default_vis),
			std::make_pair("default", visibility::default_vis),
			std::make_pair("public", visibility::pub),
			std::make_pair("private", visibility::priv),
			std::make_pair("followersonly", visibility::priv),
			std::make_pair("unlisted", visibility::unlisted),
			std::make_pair("direct", visibility::direct),
			std::make_pair("dm", visibility::direct));

		//it would be really nice if these could be vectors of string views,
		//but it's way more convenient to be able to compare the vectors with ==
		//so I'm hoping small string optimization helps here
		const auto attachments = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{u8"a 📎 attachment"},
			std::vector<std::string>{"h,i", "there"},
			std::vector<std::string>{"four", "entire", "attachments", "foryou"}
		);

		auto descriptions = GENERATE(
			std::vector<std::string>{},
			std::vector<std::string>{u8"d: a 📎 attachment"},
			std::vector<std::string>{"d: h,i", "d: there"},
			std::vector<std::string>{"d: four", "", u8"d: 🦹‍♀️ attachments", "d: foryou"}
		);

		make_file(fi.filename(), content_warning, reply_to, reply_id, visibility.first, attachments, descriptions, testtext, false);

		WHEN("A new outgoing_post is made from the same file")
		{
			{
				const outgoing_post result{ fi.filename() };

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
				const outgoing_post result{ fi.filename() };

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
		}

		WHEN("A new readonly_outgoing_post is made from the same file")
		{
			{
				readonly_outgoing_post result{ fi.filename() };

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
				result.parsed.content_warning = "more garbage";
				result.parsed.reply_to_id = "some jerk";
				result.parsed.attachments.push_back("bad attachment");
				result.parsed.descriptions.push_back("bad description");
			}

			AND_WHEN("That post is modified, destroyed and a new one is created")
			{
				const readonly_outgoing_post result{ fi.filename() };

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
					REQUIRE_FALSE(fs::exists(fi.filenamebak()));
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
