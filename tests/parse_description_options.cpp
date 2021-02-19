#include <catch2/catch.hpp>

#include "test_helpers.hpp"

#include "../console/optionparsing/parse_options.hpp"

bool flag_set(int combo, int position)
{
	return combo & (1 << position);
}

struct command_line_option
{
	std::vector<const char*> options;
	unsigned int attachment_order = 0;
	unsigned int description_order = 0;
	unsigned int order = 1000;

	friend bool operator< (const command_line_option& lhs, const command_line_option& rhs)
	{
		return lhs.order < rhs.order;
	}
};

void pick_attachment(int number, gen_options& expected, std::vector<command_line_option>& options)
{
	switch (number)
	{
	case 0:
		options.push_back(command_line_option{ {"-f", "someattach"} });
		expected.post.attachments.push_back("someattach");
		break;
	case 1:
		options.push_back(command_line_option{ {"--attach", u8"attacher🖇"}, 1 });
		options.push_back(command_line_option{ { "--attachment", "somefile"}, 2 });
		expected.post.attachments.push_back(u8"attacher🖇");
		expected.post.attachments.push_back("somefile");
		break;
	case 2:
		options.push_back(command_line_option{ {"--file", "filey"} });
		expected.post.attachments.push_back("filey");
		break;
	case 3:
		break;
	}
}

auto pick_visibility()
{
	switch (zero_to_n(7))
	{
	case 0:
		return std::make_pair("", visibility::default_vis);
	case 1:
		return std::make_pair("default", visibility::default_vis);
	case 2:
		return std::make_pair("public", visibility::pub);
	case 3:
		return std::make_pair("private", visibility::priv);
	case 4:
		return std::make_pair("followersonly", visibility::priv);
	case 5:
		return std::make_pair("unlisted", visibility::unlisted);
	case 6:
		return std::make_pair("dm", visibility::direct);
	case 7:
		return std::make_pair("direct", visibility::direct);
	}

	FAIL("Hey, the visibility picker screwed up.");
	return std::make_pair("Well, this shouldn't happen.", visibility::default_vis);
}

template <typename get_field>
bool should_reverse(const std::vector<command_line_option>& options, get_field func)
{
	const auto found = std::find_if(options.begin(), options.end(), [func](const auto& opt) { return func(opt) != 0; });
	if (found == options.end() || func(*found) == 1)
		return false;
	return true;
}

template <typename T>
std::vector<T> reversed(const std::vector<T>& vec)
{
	return std::vector<T> { vec.rbegin(), vec.rend() };
}

void check_parse(std::vector<const char*>& argv, const std::vector<command_line_option>& options, const gen_options& expected)
{
	if (flip_coin())
		argv = { "msync", "gen" };
	else
		argv = { "msync", "generate" };

	for (const auto& option : options)
	{
		argv.insert(argv.end(), option.options.begin(), option.options.end());
	}

	const auto parsed = parse((int)argv.size(), argv.data());

	THEN("the options are parsed as expected")
	{
		CAPTURE(argv);
		REQUIRE(parsed.okay);
		REQUIRE(parsed.selected == mode::gen);
		REQUIRE(parsed.account.empty());

		REQUIRE(expected.filename == parsed.gen_opt.filename);
		REQUIRE(expected.post.text == parsed.gen_opt.post.text);
		REQUIRE(expected.post.vis == parsed.gen_opt.post.vis);
		REQUIRE(expected.post.content_warning == parsed.gen_opt.post.content_warning);
		REQUIRE(expected.post.reply_to_id == parsed.gen_opt.post.reply_to_id);
		REQUIRE(expected.post.reply_id == parsed.gen_opt.post.reply_id);

		// basically, it's possible for these to get permuted so that they're not in the original order.
		if (should_reverse(options, [](const command_line_option& opt) { return opt.attachment_order; }))
			REQUIRE(reversed(expected.post.attachments) == parsed.gen_opt.post.attachments);
		else
			REQUIRE(expected.post.attachments == parsed.gen_opt.post.attachments);

		if (should_reverse(options, [](const command_line_option& opt) { return opt.description_order; }))
			REQUIRE(reversed(expected.post.descriptions) == parsed.gen_opt.post.descriptions);
		else
			REQUIRE(expected.post.descriptions == parsed.gen_opt.post.descriptions);
	}
}

void fill_options(std::vector<command_line_option>& options, gen_options& expected, const int combination, const int attach, const std::pair<const char*, visibility>& vis)
{
	if (vis.first[0] != '\0')
	{
		command_line_option opt;

		switch (zero_to_n(2))
		{
		case 0:
			opt.options.push_back("-p");
			break;
		case 1:
			opt.options.push_back("--privacy");
			break;
		case 2:
			opt.options.push_back("--visibility");
			break;
		}

		opt.options.push_back(vis.first);
		expected.post.vis = vis.second;
		options.push_back(std::move(opt));
	}

	if (attach != 3)
	{
		pick_attachment(attach, expected, options);
	}

	if (flag_set(combination, 0))
	{
		command_line_option opt;
		if (flip_coin())
			opt.options.push_back("-o");
		else
			opt.options.push_back("--output");

		opt.options.push_back("filename");
		expected.filename = "filename";
		options.push_back(std::move(opt));
	}

	if (flag_set(combination, 1))
	{
		command_line_option opt;
		if (flip_coin())
			opt.options.push_back("-r");
		else
			opt.options.push_back("--reply-to");

		opt.options.push_back("1234567");
		expected.post.reply_to_id = "1234567";
		options.push_back(std::move(opt));
	}

	if (flag_set(combination, 2))
	{
		command_line_option opt;

		switch (zero_to_n(2))
		{
		case 0:
			opt.options.push_back("-c");
			break;
		case 1:
			opt.options.push_back("--content-warning");
			break;
		case 2:
			opt.options.push_back("--cw");
			break;
		}

		opt.options.push_back("there's content in here!");
		expected.post.content_warning = "there's content in here!";
		options.push_back(std::move(opt));
	}

	if (flag_set(combination, 3))
	{
		command_line_option opt;
		if (flip_coin())
			opt.options.push_back("-i");
		else
			opt.options.push_back("--reply-id");

		opt.options.push_back("76543");
		expected.post.reply_id = "76543";
		options.push_back(std::move(opt));
	}

	if (flag_set(combination, 4))
	{
		command_line_option opt;
		switch (zero_to_n(2))
		{
		case 0:
			opt.options.push_back("-b");
			break;
		case 1:
			opt.options.push_back("--body");
			break;
		case 2:
			opt.options.push_back("--content");
			break;
		}

		opt.options.push_back("@someguy@website.com");
		expected.post.text = "@someguy@website.com";
		options.push_back(std::move(opt));
	}

	for (unsigned int i = 0; i < options.size(); i++)
		options[i].order = i;
}

void permute_and_check(std::vector<command_line_option>& options, const gen_options& expected)
{

	// static and doing the pass-by-mutable-ref thing because there's really no sense in 
	// freeing and reallocating for every test case
	// check_parse clears it every time, but keeps the capacity
	static std::vector<const char*> argv;

	// exhaustively trying every permutation takes far too long once you get past 7 or 8
	// so if there's more than that, randomly shuffle instead
	if (options.size() <= 7)
	{
		do
		{
			check_parse(argv, options, expected);
		} while (std::next_permutation(options.begin(), options.end()));
	}
	else
	{
		static std::minstd_rand g(std::random_device{}());
		// shuffle once because shuffling is slow
		std::shuffle(options.begin(), options.end(), g);
		for (int i = 0; i < 6000; i++)
		{
			check_parse(argv, options, expected);
			std::next_permutation(options.begin(), options.end());
		}
	}
}

SCENARIO("The command line parser recognizes when the user wants to generate a file with a single description.", "[long_run][long_run_parseopts]")
{
	GIVEN("A combination of options for the file generator")
	{
		// try every combination of bits. note that the ranges are half-open, including the 0 and excluding the maximum.
		// this test isn't as exhaustive as it could be, because if it was, it'd take forever to run
		const auto combination = GENERATE(range(0, 0b11111 + 1));
		const auto attach = GENERATE(0, 1, 2, 3);
		const auto vis = pick_visibility();

		gen_options expected;

		// this guy is going to be refilled and emptied a bunch
		// make 'em static and clear it every time to keep the capacity
		static std::vector<command_line_option> options;
		options.clear();

		options.push_back(command_line_option{ {"-d", "some🕳descrip"} });
		expected.post.descriptions.push_back("some🕳descrip");

		fill_options(options, expected, combination, attach, vis);

		WHEN("the command line is parsed")
		{
			permute_and_check(options, expected);
		}
	}
}

SCENARIO("The command line parser recognizes when the user wants to generate a file with two descriptions.", "[long_run][long_run_parseopts]")
{
	GIVEN("A combination of options for the file generator")
	{
		// try every combination of bits. note that the ranges are half-open, including the 0 and excluding the maximum.
		// this test isn't as exhaustive as it could be, because if it was, it'd take forever to run
		const auto combination = GENERATE(range(0, 0b11111 + 1));
		const auto attach = GENERATE(0, 1, 2, 3);
		const auto vis = pick_visibility();

		gen_options expected;

		// this guy is going to be refilled and emptied a bunch
		// make 'em static and clear it every time to keep the capacity
		static std::vector<command_line_option> options;
		options.clear();

		options.push_back(command_line_option{ {"--description", "describer"}, 0, 1 });
		options.push_back(command_line_option{ {"-d", "some file!"}, 0, 2 });
		expected.post.descriptions.push_back("describer");
		expected.post.descriptions.push_back("some file!");

		fill_options(options, expected, combination, attach, vis);

		WHEN("the command line is parsed")
		{
			permute_and_check(options, expected);
		}
	}
}


SCENARIO("The command line parser recognizes when the user wants to generate a file without any descriptions.", "[long_run][long_run_parseopts]")
{
	GIVEN("A combination of options for the file generator")
	{
		// try every combination of bits. note that the ranges are half-open, including the 0 and excluding the maximum.
		// this test isn't as exhaustive as it could be, because if it was, it'd take forever to run
		const auto combination = GENERATE(range(0, 0b11111 + 1));
		const auto attach = GENERATE(0, 1, 2, 3);
		const auto vis = pick_visibility();

		gen_options expected;

		// this guy is going to be refilled and emptied a bunch
		// make 'em static and clear it every time to keep the capacity
		static std::vector<command_line_option> options;
		options.clear();

		fill_options(options, expected, combination, attach, vis);

		WHEN("the command line is parsed")
		{
			permute_and_check(options, expected);
		}
	}
}
