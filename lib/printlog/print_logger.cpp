#include "print_logger.hpp"

bool verbose_logs = false;
bool logs_off = false;

std::ofstream& logfile()
{
	static std::ofstream of("msync.log", std::ios::out | std::ios::app);
	return of;
}

print_logger<logtype::normal>& pl()
{
	static print_logger<logtype::normal> pl(logfile());
	return pl;
}

print_logger<logtype::verbose>& plverb()
{
	static print_logger<logtype::verbose> pl(logfile());
	return pl;
}

print_logger<logtype::fileonly>& plfile()
{
	static print_logger<logtype::fileonly> pl(logfile());
	return pl;
}
