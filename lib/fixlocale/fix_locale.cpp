#include "fix_locale.hpp"

#include <locale>
#include <stdexcept>

void fix_locale()
{
	//mostly for Windows so it interprets strings of type char as UTF-8 when constructing paths.
	//pre-10 Windows doesn't really have UTF-8 support, so just try to do it and fail silently
	try
	{
		std::locale::global(std::locale("en_US.UTF-8"));
	}
	catch (std::runtime_error) { }
}