#include "fix_locale.hpp"

#ifdef _WIN32
#include <locale>
#include <stdexcept>
#include <algorithm>
#include <cctype>

bool is_utf8(std::string& localename)
{
	std::transform(localename.begin(), localename.end(), localename.begin(),
		[](unsigned char c) -> unsigned char { return std::toupper(c); });

	for (const auto utf8 : { "UTF-8", "UTF8", "65001" })
	{
		if (localename.find(utf8) != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

void fix_locale()
{
	//mostly for Windows so it interprets strings of type char as UTF-8 when constructing paths.
	//pre-10 Windows doesn't really have UTF-8 support, so just try to do it and fail silently

	// later: see if I can just add a utf-8 codecvt facet to the current locale
	// also, write a test for this
	auto def = std::locale("");
	const auto& def_codecvt = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(def);
	if (!is_utf8(def.name()))
	{
		try
		{
			auto loc = std::locale(".65001");
			const auto& codecvt = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc);
			std::locale::global(loc);
		}
		catch (const std::runtime_error&) {}
	}
	
}
#else
void fix_locale(){}
#endif
