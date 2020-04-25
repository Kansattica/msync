#include <catch2/catch.hpp>

#include "../lib/sync/send.hpp"
#include "../lib/netinterface/net_interface.hpp"
#include "../lib/queue/queues.hpp"
#include "../lib/constants/constants.hpp"

#include "test_helpers.hpp"
#include "mock_network.hpp"

#include <string_view>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <print_logger.hpp>

struct id_mock_args : public basic_mock_args
{
	std::string id;
};

unsigned int sequence = 0;

struct mock_network_post : public mock_network
{
	std::vector<basic_mock_args> arguments;

	net_response operator()(std::string_view url, std::string_view access_token)
	{
		arguments.push_back(basic_mock_args{ ++sequence, std::string {url}, std::string { access_token } });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		if (!toreturn.okay)
			toreturn.message = R"({ "error": "some problem" })";
		return toreturn;
	}
};

struct mock_network_delete : public mock_network
{
	std::vector<basic_mock_args> arguments;

	net_response operator()(std::string_view url, std::string_view access_token)
	{
		arguments.push_back(basic_mock_args{ ++sequence, std::string {url}, std::string { access_token } });

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		if (!toreturn.okay)
			toreturn.message = R"({ "error": "some problem" })";
		return toreturn;
	}
};

struct status_mock_args : public id_mock_args
{
	status_params params;
};

struct mock_network_new_status : public mock_network
{
	std::string fail_if_body;

	std::vector<status_mock_args> arguments;

	net_response operator()(std::string_view url, std::string_view access_token, const status_params& params)
	{
		if (!fail_if_body.empty())
			fatal_error = fail_if_body == params.body;

		static unsigned int id = 1000000;
		std::string str_id = std::to_string(++id);

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		if (!toreturn.okay)
			toreturn.message = R"({ "error": "some problem" })";
		else
			make_status_json(str_id, toreturn.message);

		arguments.push_back(status_mock_args{{{++sequence, std::string {url}, std::string { access_token }}, std::move(str_id)}, params });

		return toreturn;
	}
};

struct upload_mock_args : public id_mock_args
{
	attachment attachment_args;
};

struct mock_network_upload : public mock_network
{
	std::vector<upload_mock_args> arguments;
	net_response operator()(std::string_view url, std::string_view access_token, const fs::path& file, const std::string& description)
	{
		static unsigned int id = 100;
		std::string str_id = std::to_string(++id);

		net_response toreturn;
		toreturn.retryable_error = (--succeed_after > 0);
		if (succeed_after == 0) { succeed_after = succeed_after_n; }
		toreturn.okay = !(fatal_error || toreturn.retryable_error);
		toreturn.status_code = status_code;
		toreturn.message = R"({"id": ")";
		toreturn.message += str_id;
		toreturn.message += "\"}";

		arguments.push_back(upload_mock_args{ {{++sequence, std::string {url}, std::string { access_token }}, std::move(str_id) },
			attachment{file, description} });

		return toreturn;
	}
};

std::string make_expected_url(const std::string_view id, const std::string_view route, const std::string_view instance_url)
{
	std::string toreturn{ "https://" };
	toreturn.append(instance_url).append("/api/v1/statuses/").append(id).append(route);
	return toreturn;
}

std::vector<std::string_view> repeat_each_element(const std::vector<std::string>& in, size_t count)
{
	std::vector<std::string_view> toreturn;
	for (const auto& str : in)
	{
		for (size_t i = 0; i < count; i++)
		{
			toreturn.emplace_back(str);
		}
	}
	return toreturn;
}

SCENARIO("Send correctly sends from and modifies the queue with favs and boosts.")
{
	logs_off = true;

	const test_dir testdir = temporary_directory();
	const fs::path account = testdir.dirname / "someguy@cool.account";
	fs::create_directory(account); //enqueue expects this directory to exist already

	constexpr std::string_view instanceurl = "cool.account";
	constexpr std::string_view accesstoken = "sometoken";

	const auto queue = GENERATE(
		std::make_tuple(queues::fav, "/favourite", "/unfavourite", "FAV "),
		std::make_tuple(queues::boost, "/reblog", "/unreblog", "BOOST "));

	const std::vector<std::string> testvect = GENERATE(
		std::vector<std::string>{ "someid", "someotherid", "mrid" },
		std::vector<std::string>{},
		std::vector<std::string>{ "justone" });

	GIVEN("A queue with some ids to add and a good connection")
	{
		enqueue(std::get<0>(queue), account, std::vector<std::string>{testvect});

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mock_network_delete mockdel;
			mock_network_new_status mocknew;
			mock_network_upload mockupload;

			auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(testvect.size() == mockpost.arguments.size());
				for (unsigned int i = 0; i < testvect.size(); i++)
				{
					REQUIRE(mockpost.arguments[i].url == make_expected_url(testvect[i], std::get<1>(queue), instanceurl));
				}
			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
				REQUIRE(mockupload.arguments.empty());
			}
		}
	}

	GIVEN("A queue with some ids to remove and a good connection")
	{
		dequeue(std::get<0>(queue), account, std::vector<std::string>{testvect});

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mock_network_delete mockdel;
			mock_network_new_status mocknew;
			mock_network_upload mockupload;

			auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(account).empty());
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(testvect.size() == mockpost.arguments.size());
				for (unsigned int i = 0; i < testvect.size(); i++)
				{
					REQUIRE(mockpost.arguments[i].url == make_expected_url(testvect[i], std::get<2>(queue), instanceurl));
				}

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
				REQUIRE(mockupload.arguments.empty());
			}
		}
	}

	GIVEN("A queue with some ids to add and retryable errors that ultimately succeed")
	{
		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		const auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));

		enqueue(std::get<0>(queue), account, std::vector<std::string>{testvect});

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mockpost.set_succeed_after(retries.second);

			mock_network_delete mockdel;
			mock_network_new_status mocknew;
			mock_network_upload mockupload;

			auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue is now empty.")
			{
				REQUIRE(print(account).empty());
			}

			THEN("each ID was tried the correct number of times.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size() * retries.second);
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				const auto repeated = repeat_each_element(testvect, retries.second);
				REQUIRE(repeated.size() == mockpost.arguments.size());
				for (unsigned int i = 0; i < repeated.size(); i++)
				{
					REQUIRE(mockpost.arguments[i].url == make_expected_url(repeated[i], std::get<1>(queue), instanceurl));
				}

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
				REQUIRE(mockupload.arguments.empty());
			}
		}
	}

	GIVEN("A queue where all the IDs fail")
	{
		// first is the number of retries to feed to send
		// second is the number of retries to expect
		// the last two test the "if retries less than 1, set to 3" behavior
		const auto retries = GENERATE(
			std::make_pair(3, 3),
			std::make_pair(5, 5),
			std::make_pair(1, 1),
			std::make_pair(0, 3),
			std::make_pair(-1, 3));


		enqueue(std::get<0>(queue), account, std::vector<std::string>{testvect});

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mockpost.fatal_error = true;
			mockpost.status_code = 500;

			mock_network_delete mockdel;
			mock_network_new_status mocknew;
			mock_network_upload mockupload;

			auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue hasn't changed.")
			{
				REQUIRE(print(account) == make_expected_ids(testvect, std::get<3>(queue)));
			}

			THEN("each ID was tried once.")
			{
				REQUIRE(mockpost.arguments.size() == testvect.size());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(testvect.size() == mockpost.arguments.size());
				for (unsigned int i = 0; i < testvect.size(); i++)
				{
					REQUIRE(mockpost.arguments[i].url == make_expected_url(testvect[i], std::get<1>(queue), instanceurl));
				}

			}

			THEN("only the post function was called.")
			{
				REQUIRE(mockdel.arguments.empty());
				REQUIRE(mocknew.arguments.empty());
				REQUIRE(mockupload.arguments.empty());
			}
		}
	}
}

//ensures a file only exists during each test run
struct touch_file
{
public:
	touch_file(const char* name) : touch_file(fs::path(name)) {};
	touch_file(fs::path name) : filename(std::move(name))
	{
		std::ofstream of(filename.c_str(), std::ios::out | std::ios::app);
	};

	~touch_file()
	{
		fs::remove(filename);
	};

	const fs::path filename;
};

int unique_idempotency_keys(std::initializer_list<uint_fast64_t> keys)
{
	int number_of_tests = 0;
	for (auto first = keys.begin(); first != keys.end(); ++first)
	{
		for (auto next = first + 1; next != keys.end(); ++next)
		{
			REQUIRE(*first != *next);
			++number_of_tests;
		}
	}
	return number_of_tests;
}

SCENARIO("Send correctly sends new posts and deletes existing ones.")
{
	logs_off = true;
	const test_dir dir = temporary_directory();

	const fs::path account = dir.dirname / "someguy@cool.account";

	constexpr std::string_view instanceurl = "cool.account";
	constexpr std::string_view accesstoken = "sometoken";
	constexpr std::string_view new_post_url = "https://cool.account/api/v1/statuses";

	const fs::path queue_directory = account / File_Queue_Directory;
	fs::create_directories(queue_directory);

	GIVEN("A queue with some post filenames to send.")
	{
		const std::array<test_file, 4> to_enqueue { "first.post", "second.post", "another kind of post", "last one" };
		const std::array<touch_file, 4> attachment_files{ "attachments", "on", "this", "one" };

		const static std::vector<fs::path> expected_attach{ fs::canonical("attachments"), fs::canonical("on")
			, fs::canonical("this"), fs::canonical("one") };
		const static std::vector<std::string> expected_descriptions{ "with", "some", "descriptions", "" };
		const static std::vector<std::string> expected_files{ "first.post", "second.post", "another kind of post", "last one" };

		{
			outgoing_post first{ to_enqueue[0].filename };
			first.parsed.text = "This one just has a body.";
			first.parsed.reply_id = "Hi";

			outgoing_post second{ to_enqueue[1].filename };
			second.parsed.text = "This one has a body, too.";
			second.parsed.content_warning = "And a content warning.";
			second.parsed.vis = visibility::priv;
			second.parsed.reply_id = "hi2hi";
			second.parsed.reply_to_id = "Hi";

			outgoing_post third{ to_enqueue[2].filename };
			third.parsed.attachments = { "attachments", "on" };
			third.parsed.descriptions = { "with", "some", "descriptions" };
			third.parsed.reply_to_id = "hi2hi";
			third.parsed.vis = visibility::direct;

			outgoing_post fourth{ to_enqueue[3].filename };
			fourth.parsed.attachments = { "attachments", "on", "this", "one" };
			fourth.parsed.descriptions = { "with", "some", "descriptions" };
			fourth.parsed.reply_to_id = "777777";
			fourth.parsed.vis = visibility::unlisted;
		}

		enqueue(queues::post, account, std::vector<std::string>{expected_files});

		mock_network_post mockpost;
		mock_network_delete mockdel;
		mock_network_new_status mocknew;
		mock_network_upload mockupload;

		auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

		WHEN("the posts are sent over a good connection")
		{
			send.send(account, instanceurl, accesstoken);

			THEN("the queue and post directory is now empty.")
			{
				REQUIRE(print(account).empty());

				// it'll leave the .bak files behind
				REQUIRE(count_files_in_directory(queue_directory) == 4);
			}

			THEN("the input files and attachments are untouched")
			{
				REQUIRE(std::all_of(to_enqueue.begin(), to_enqueue.end(), [](const auto& file) { return fs::exists(file.filename); }));
				REQUIRE(std::all_of(attachment_files.begin(), attachment_files.end(), [](const auto& file) { return fs::exists(file.filename); }));
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mocknew.arguments.size() == 4);
			}

			THEN("the other APIs weren't called.")
			{
				REQUIRE(mockpost.arguments.empty());
				REQUIRE(mockdel.arguments.empty());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mocknew.arguments.begin(), mocknew.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.url == new_post_url; }));
			}

			THEN("the post parameters are as expected.")
			{
				const auto& first = mocknew.arguments[0];
				REQUIRE(first.params.attachment_ids.empty());
				REQUIRE(first.params.body == "This one just has a body.");
				REQUIRE(first.params.content_warning.empty());
				REQUIRE(first.params.reply_to.empty());
				REQUIRE(first.params.visibility.empty());

				const auto& second = mocknew.arguments[1];
				REQUIRE(second.params.attachment_ids.empty());
				REQUIRE(second.params.body == "This one has a body, too.");
				REQUIRE(second.params.content_warning == "And a content warning.");
				REQUIRE(second.params.visibility == "private");
				REQUIRE(second.params.reply_to == first.id);

				const auto& third = mocknew.arguments[2];
				REQUIRE(third.params.attachment_ids.size() == 2);
				REQUIRE(third.params.body.empty());
				REQUIRE(third.params.content_warning.empty());
				REQUIRE(third.params.visibility == "direct");
				REQUIRE(third.params.reply_to == second.id);

				const auto& fourth = mocknew.arguments[3];
				REQUIRE(fourth.params.attachment_ids.size() == 4);
				REQUIRE(fourth.params.body.empty());
				REQUIRE(fourth.params.content_warning.empty());
				REQUIRE(fourth.params.reply_to == "777777");
				REQUIRE(fourth.params.visibility == "unlisted");

				REQUIRE(6 == unique_idempotency_keys({ first.params.idempotency_key, second.params.idempotency_key, third.params.idempotency_key, fourth.params.idempotency_key }));
			}


			THEN("the uploads are as expected.")
			{
				REQUIRE(mockupload.arguments.size() == 6);
				// attached to the third 
				REQUIRE(mockupload.arguments[0].attachment_args.file == expected_attach[0]);
				REQUIRE(mockupload.arguments[0].attachment_args.description == expected_descriptions[0]);
				REQUIRE(mockupload.arguments[1].attachment_args.file == expected_attach[1]);
				REQUIRE(mockupload.arguments[1].attachment_args.description == expected_descriptions[1]);
				
				//attached to the fourth
				for (size_t i = 0; i < 4; i++)
				{
					REQUIRE(mockupload.arguments[i + 2].attachment_args.file == expected_attach[i]);
					REQUIRE(mockupload.arguments[i + 2].attachment_args.description == expected_descriptions[i]);
				}
			}

		}

		WHEN("one of the threaded posts fails to send")
		{
			mocknew.fail_if_body = "This one has a body, too.";

			send.send(account, instanceurl, accesstoken);

			THEN("the queue and post directory removes the successfully sent posts.")
			{
				REQUIRE(print(account) == std::vector<std::string>{ "POST second.post", "POST another kind of post" });

				// 4 bak files, 2 regular
				REQUIRE(count_files_in_directory(queue_directory) == 6);
			}

			THEN("the input files and attachments are untouched")
			{
				REQUIRE(std::all_of(to_enqueue.begin(), to_enqueue.end(), [](const auto& file) { return fs::exists(file.filename); }));
				REQUIRE(std::all_of(attachment_files.begin(), attachment_files.end(), [](const auto& file) { return fs::exists(file.filename); }));
			}

			THEN("one call per ID was made.")
			{
				// send first OK
				// try and fail to send second
				// don't try to send third
				// send fourth OK
				REQUIRE(mocknew.arguments.size() == 3);
			}

			THEN("the other APIs weren't called.")
			{
				REQUIRE(mockpost.arguments.empty());
				REQUIRE(mockdel.arguments.empty());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mocknew.arguments.begin(), mocknew.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.url == new_post_url; }));
			}

			THEN("the post parameters are as expected.")
			{
				const auto& first = mocknew.arguments[0];
				REQUIRE(first.params.attachment_ids.empty());
				REQUIRE(first.params.body == "This one just has a body.");
				REQUIRE(first.params.content_warning.empty());
				REQUIRE(first.params.reply_to.empty());
				REQUIRE(first.params.visibility.empty());

				const auto& second = mocknew.arguments[1];
				REQUIRE(second.params.attachment_ids.empty());
				REQUIRE(second.params.body == "This one has a body, too.");
				REQUIRE(second.params.content_warning == "And a content warning.");
				REQUIRE(second.params.visibility == "private");
				REQUIRE(second.params.reply_to == first.id);

				// third should get skipped because second will fail to send

				const auto& fourth = mocknew.arguments[2];
				REQUIRE(fourth.params.attachment_ids.size() == 4);
				REQUIRE(fourth.params.body.empty());
				REQUIRE(fourth.params.content_warning.empty());
				REQUIRE(fourth.params.reply_to == "777777");
				REQUIRE(fourth.params.visibility == "unlisted");

				REQUIRE(3 == unique_idempotency_keys({ first.params.idempotency_key, second.params.idempotency_key, fourth.params.idempotency_key }));
			}

			THEN("the uploads are as expected.")
			{
				REQUIRE(mockupload.arguments.size() == 4);
				
				//attached to the fourth
				for (size_t i = 0; i < 4; i++)
				{
					REQUIRE(mockupload.arguments[i].attachment_args.file == expected_attach[i]);
					REQUIRE(mockupload.arguments[i].attachment_args.description == expected_descriptions[i]);
				}
			}

			THEN("all replies to a threaded post that fail to send persist the successful post's ID")
			{
				const auto& post_id = mocknew.arguments[0].id;
				readonly_outgoing_post failed_post{ queue_directory / "second.post" };
				REQUIRE(failed_post.parsed.reply_to_id == post_id);
			}

		}

		WHEN("the posts are sent over a bad or no connection")
		{
			mocknew.fatal_error = true;
			mocknew.status_code = 500;

			mockupload.fatal_error = true;
			mockupload.status_code = 500;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue and post directories are not empty.")
			{
				REQUIRE(print(account) == make_expected_ids(expected_files, "POST "));

				// queueing the posts makes a .bak file for them
				REQUIRE(count_files_in_directory(queue_directory) == 8);
			}

			THEN("the input files and attachments are untouched")
			{
				REQUIRE(std::all_of(to_enqueue.begin(), to_enqueue.end(), [](const auto& file) { return fs::exists(file.filename); }));
				REQUIRE(std::all_of(attachment_files.begin(), attachment_files.end(), [](const auto& file) { return fs::exists(file.filename); }));
			}

			THEN("one call per ID was made.")
			{
				REQUIRE(mocknew.arguments.size() == 1);
			}

			THEN("the other APIs weren't called.")
			{
				REQUIRE(mockpost.arguments.empty());
				REQUIRE(mockdel.arguments.empty());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mocknew.arguments.begin(), mocknew.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.url == new_post_url; }));
			}

			THEN("the post parameters are as expected.")
			{
				const auto& first = mocknew.arguments[0];
				REQUIRE(first.params.attachment_ids.empty());
				REQUIRE(first.params.body == "This one just has a body.");
				REQUIRE(first.params.content_warning.empty());
				REQUIRE(first.params.visibility.empty());

				// the second call should never be made because it was a reply to the first.

				// the third and fourth calls should never be made because the uploads failed.

			}

			THEN("Only the first upload per post is attempted.")
			{
				REQUIRE(mockupload.arguments.size() == 1);
				REQUIRE(mockupload.arguments[0].attachment_args.file == expected_attach[0]);
				REQUIRE(mockupload.arguments[0].attachment_args.description == expected_descriptions[0]);
			}


		}

		WHEN("the posts are sent over a flaky connection that eventually succeeds")
		{
			const std::pair<int, size_t> retries = GENERATE(
				std::make_pair(3, 3),
				std::make_pair(5, 5),
				std::make_pair(1, 1),
				std::make_pair(0, 3),
				std::make_pair(-1, 3));

			mocknew.fatal_error = false;
			mocknew.set_succeed_after(retries.second);

			mockupload.fatal_error = false;
			mockupload.set_succeed_after(retries.second);

			send.retries = retries.first;

			send.send(account, instanceurl, accesstoken);

			THEN("the queue and post directories are emptied.")
			{
				REQUIRE(print(account).empty());

				// queueing the posts makes a .bak file for them
				REQUIRE(count_files_in_directory(queue_directory) == 4);
			}

			THEN("the input files and attachments are untouched")
			{
				REQUIRE(std::all_of(to_enqueue.begin(), to_enqueue.end(), [](const auto& file) { return fs::exists(file.filename); }));
				REQUIRE(std::all_of(attachment_files.begin(), attachment_files.end(), [](const auto& file) { return fs::exists(file.filename); }));
			}

			THEN("[retries] calls per ID was made.")
			{
				REQUIRE(mocknew.arguments.size() == 4 * retries.second);
			}

			THEN("the other APIs weren't called.")
			{
				REQUIRE(mockpost.arguments.empty());
				REQUIRE(mockdel.arguments.empty());
			}

			THEN("the access token was passed in.")
			{
				REQUIRE(std::all_of(mocknew.arguments.begin(), mocknew.arguments.end(), [&](const auto& actual) { return actual.access_token == accesstoken; }));
			}

			THEN("the URLs are as expected.")
			{
				REQUIRE(std::all_of(mockpost.arguments.begin(), mockpost.arguments.end(), [&](const auto& actual) { return actual.url == new_post_url; }));
			}

			THEN("the post parameters are as expected.")
			{
				size_t idx = 0;

				auto expected_idempotency_key = mocknew.arguments[idx].params.idempotency_key;
				for (size_t i = 0; i < retries.second; i++)
				{
					const auto& first = mocknew.arguments[idx++];
					REQUIRE(first.params.attachment_ids.empty());
					REQUIRE(first.params.body == "This one just has a body.");
					REQUIRE(first.params.content_warning.empty());
					REQUIRE(first.params.reply_to.empty());
					REQUIRE(first.params.visibility.empty());

					// their idempotency keys should all be the same for each unique post
					REQUIRE(first.params.idempotency_key == expected_idempotency_key);
				}

				// the other replies will be to the last ID for this post
				// since that's the one that actually went through
				std::string reply_to_id = mocknew.arguments[idx - 1].id;

				// keys should be different between posts
				REQUIRE(expected_idempotency_key != mocknew.arguments[idx].params.idempotency_key);
				expected_idempotency_key = mocknew.arguments[idx].params.idempotency_key;

				for (size_t i = 0; i < retries.second; i++)
				{
					const auto& second = mocknew.arguments[idx++];
					REQUIRE(second.params.attachment_ids.empty());
					REQUIRE(second.params.body == "This one has a body, too.");
					REQUIRE(second.params.content_warning == "And a content warning.");
					REQUIRE(second.params.visibility == "private");
					REQUIRE(second.params.idempotency_key == expected_idempotency_key);

					// test that posts are threaded correctly
					REQUIRE(second.params.reply_to == reply_to_id);
				}

				// do it again because third is a reply to second
				reply_to_id = mocknew.arguments[idx - 1].id;

				REQUIRE(expected_idempotency_key != mocknew.arguments[idx].params.idempotency_key);
				expected_idempotency_key = mocknew.arguments[idx].params.idempotency_key;

				for (size_t i = 0; i < retries.second; i++)
				{
					const auto& third = mocknew.arguments[idx++];
					REQUIRE(third.params.attachment_ids.size() == 2);
					REQUIRE(third.params.body.empty());
					REQUIRE(third.params.content_warning.empty());
					REQUIRE(third.params.visibility == "direct");
					REQUIRE(third.params.reply_to == reply_to_id);
					REQUIRE(third.params.idempotency_key == expected_idempotency_key);
				}

				REQUIRE(expected_idempotency_key != mocknew.arguments[idx].params.idempotency_key);
				expected_idempotency_key = mocknew.arguments[idx].params.idempotency_key;

				for (size_t i = 0; i < retries.second; i++)
				{
					const auto& fourth = mocknew.arguments[idx++];
					REQUIRE(fourth.params.attachment_ids.size() == 4);
					REQUIRE(fourth.params.body.empty());
					REQUIRE(fourth.params.content_warning.empty());
					REQUIRE(fourth.params.reply_to == "777777");
					REQUIRE(fourth.params.visibility == "unlisted");
					REQUIRE(fourth.params.idempotency_key == expected_idempotency_key);
				}
			}

			THEN("the uploads are as expected.")
			{
				REQUIRE(mockupload.arguments.size() == 6 * retries.second);

				size_t idx = 0;

				// attached to the third 
				for (size_t i = 0; i < retries.second; i++)
				{
					REQUIRE(mockupload.arguments[idx].attachment_args.file == expected_attach[0]);
					REQUIRE(mockupload.arguments[idx].attachment_args.description == expected_descriptions[0]);
					idx++;
				}

				for (size_t i = 0; i < retries.second; i++)
				{
					REQUIRE(mockupload.arguments[idx].attachment_args.file == expected_attach[1]);
					REQUIRE(mockupload.arguments[idx].attachment_args.description == expected_descriptions[1]);
					idx++;
				}

				//attached to the fourth
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < retries.second; j++)
					{
						REQUIRE(mockupload.arguments[idx].attachment_args.file == expected_attach[i]);
						REQUIRE(mockupload.arguments[idx].attachment_args.description == expected_descriptions[i]);
						idx++;
					}
				}
			}
		}
	}
}

SCENARIO("Send correctly sends from and modifies a queue of mixed API calls.")
{
	logs_off = true;

	const test_dir dir = temporary_directory();
	const fs::path account = dir.dirname / "prettynormal@website.egg";
	fs::create_directory(account);
	constexpr std::string_view instanceurl = "website.egg";
	constexpr std::string_view accesstoken = "someothertoken";

	GIVEN("A queue with some ids to add and a good connection")
	{
		dequeue(queues::boost, account, { "worstpost" });
		enqueue(queues::boost, account, { "somekindapost", "anotherkindapost" });
		enqueue(queues::fav, account, { "somekindapost", "mrs. goodpost" });
		dequeue(queues::post, account, { "real stinker" });
		dequeue(queues::fav, account, { "badpost" });

		WHEN("the queue is sent")
		{
			mock_network_post mockpost;
			mock_network_delete mockdel;
			mock_network_new_status mocknew;
			mock_network_upload mockupload;

			auto send = send_posts{ mockpost, mockdel, mocknew, mockupload };

			sequence = 0;
			send.send(account, instanceurl, accesstoken);

			THEN("the queue is empty.")
			{
				REQUIRE(read_file(account / Queue_Filename).empty());
			}

			THEN("the correct number of calls were made.")
			{
				REQUIRE(mockpost.arguments.size() == 6);
				REQUIRE(mockdel.arguments.size() == 1);
				REQUIRE(mocknew.arguments.size() == 0);
				REQUIRE(mockupload.arguments.size() == 0);
			}

			THEN("the correct calls were made")
			{
				REQUIRE(mockpost.arguments[0].access_token == accesstoken);
				REQUIRE(mockpost.arguments[0].sequence == 1);
				REQUIRE(mockpost.arguments[0].url == make_expected_url("worstpost", "/unreblog", instanceurl));

				REQUIRE(mockpost.arguments[1].access_token == accesstoken);
				REQUIRE(mockpost.arguments[1].sequence == 2);
				REQUIRE(mockpost.arguments[1].url == make_expected_url("somekindapost", "/reblog", instanceurl));

				REQUIRE(mockpost.arguments[2].access_token == accesstoken);
				REQUIRE(mockpost.arguments[2].sequence == 3);
				REQUIRE(mockpost.arguments[2].url == make_expected_url("anotherkindapost", "/reblog", instanceurl));

				REQUIRE(mockpost.arguments[3].access_token == accesstoken);
				REQUIRE(mockpost.arguments[3].sequence == 4);
				REQUIRE(mockpost.arguments[3].url == make_expected_url("somekindapost", "/favourite", instanceurl));

				REQUIRE(mockpost.arguments[4].access_token == accesstoken);
				REQUIRE(mockpost.arguments[4].sequence == 5);
				REQUIRE(mockpost.arguments[4].url == make_expected_url("mrs. goodpost", "/favourite", instanceurl));

				REQUIRE(mockdel.arguments[0].access_token == accesstoken);
				REQUIRE(mockdel.arguments[0].sequence == 6);
				REQUIRE(mockdel.arguments[0].url == make_expected_url("real stinker", "", instanceurl));

				REQUIRE(mockpost.arguments[5].access_token == accesstoken);
				REQUIRE(mockpost.arguments[5].sequence == 7);
				REQUIRE(mockpost.arguments[5].url == make_expected_url("badpost", "/unfavourite", instanceurl));

			}
		}
	}
}

SCENARIO("read_params doesn't repeat idempotency keys or mutate the post file.")
{
	const test_file fi = temporary_file();
	GIVEN("An outgoing post to read from.")
	{
		constexpr std::string_view expected_text = "hi there, bud";
		constexpr std::string_view expected_cw = "careful, watch out for that content";
		constexpr std::string_view visibility = "direct";
		
		{
			outgoing_post towrite{ fi.filename };
			towrite.parsed.text = expected_text;
			towrite.parsed.content_warning = expected_cw;
			towrite.parsed.vis = visibility::direct;
		}

		WHEN("The outgoing_post is read by read_params repeatedly.")
		{
			constexpr int trials = 50000;
			std::vector<uint_fast64_t> seen_ids(trials);
			THEN("The paramaters are always as expected, and the idempotency_ids never repeat.")
			{
				for (int i = 0; i < trials; i++)
				{
					const auto params = read_params(fi.filename);
					REQUIRE(params.attachments.empty());
					REQUIRE(params.attachment_ids.empty());
					REQUIRE(params.body == expected_text);
					REQUIRE(params.content_warning == expected_cw);
					REQUIRE(params.okay);
					REQUIRE(params.reply_id.empty());
					REQUIRE(params.reply_to.empty());
					REQUIRE(params.visibility == visibility);

					seen_ids[i] = params.idempotency_key;
				}

				std::sort(seen_ids.begin(), seen_ids.end());
				REQUIRE(seen_ids[0] != 0);
				REQUIRE(std::adjacent_find(seen_ids.begin(), seen_ids.end()) == seen_ids.end());
			}
		}
	}
}

