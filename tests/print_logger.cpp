#include <catch2/catch.hpp>

#include <print_logger.hpp>
#include <constants.hpp>
#include <filesystem.hpp>

SCENARIO("The print logger respects the MSYNC_FILE_LOG define.")
{
	GIVEN("A print logger that's turned on.")
	{
		logs_off = false;
		verbose_logs = false;

		WHEN("Some messages are logged.")
		{
			plverb() << "One message.";
			plfile() << "Another message.";

#ifdef MSYNC_FILE_LOG
			constexpr bool should_exist = true;
			THEN("msync.log exists.")
#else
			constexpr bool should_exist = false;
			THEN("msync.log doesn't exist.")
#endif
			{
				REQUIRE(fs::exists("msync.log") == should_exist);
			}
		}

		logs_off = true;
	}
}