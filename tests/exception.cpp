#include <catch2/catch.hpp>

#include <msync_exception.hpp>
#include <string>

void throw_string_ex(const std::string& message)
{
	throw msync_exception(message);
}

void throw_char_ex(const char* message)
{
	throw msync_exception(message);
}

SCENARIO("msync_exception can be thrown and caught properly with both string and char* messages.")
{
	GIVEN("A message in the exception to be thrown.")
	{
		const char* message = GENERATE("", "this is a test", "Hello!", "had a problem in the place here: everywhere");

		WHEN("A string exception is thrown.")
		{
			THEN("The message is as expected.")
			{
				REQUIRE_THROWS_WITH(throw_string_ex(message), message);
				REQUIRE_THROWS_AS(throw_string_ex(message), msync_exception);
			}
		}

		WHEN("A char* exception is thrown.")
		{
			THEN("The message is as expected.")
			{
				REQUIRE_THROWS_WITH(throw_char_ex(message), message);
				REQUIRE_THROWS_AS(throw_char_ex(message), msync_exception);
			}
		}
	}
}
