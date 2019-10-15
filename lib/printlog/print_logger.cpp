#include "print_logger.hpp"

bool verbose_logs = false;
bool logs_off = false;

print_logger<logtype::normal>& pl()
{
	static print_logger<logtype::normal> pl;
	return pl;
}

print_logger<logtype::verbose>& plverb()
{
	static print_logger<logtype::verbose> pl;
	return pl;
}

print_logger<logtype::fileonly>& plfile()
{
	static print_logger<logtype::fileonly> pl;
	return pl;
}
