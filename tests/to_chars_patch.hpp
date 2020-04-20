#ifndef TO_CHARS_PATCH
#define TO_CHARS_PATCH

#include <charconv>
#include <catch2/catch.hpp>

// apple's stdlib is missing to_chars
#if __APPLE__ && !defined(__cpp_lib_to_chars)
#include <cstdio>
#endif

template <typename Number>
std::string_view sv_to_chars(Number n, std::array<char, 10>& char_buf)
{
	// see https://en.cppreference.com/w/cpp/utility/to_chars
	// this avoids an allocation compared to std::to_string

	// note that this function takes a character buffer that it will clobber and returns a string view into it
	// this is to avoid allocations and also not return pointers into memory that will be freed when the function returns.

#if __APPLE__ && !defined(__cpp_lib_to_chars)
	const int written = sprintf(char_buf.data(), "%u", n);
	if (written > 10) { FAIL("You messed up with sprintf, ya dingus."); }
	return std::string_view(char_buf.data(), written);
#else
	const auto [end, err] = std::to_chars(char_buf.data(), char_buf.data() + char_buf.size(), n);
	if (err != std::errc()) { FAIL("You messed up with to_chars, ya dingus."); }
	return std::string_view(char_buf.data(), end - char_buf.data());
#endif
}

#endif
