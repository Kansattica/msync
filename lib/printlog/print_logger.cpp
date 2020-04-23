#include "print_logger.hpp"
#include <constants.hpp>

bool verbose_logs = false;
bool logs_off = false;

#ifdef MSYNC_FILE_LOG
std::ofstream logfile("msync.log", std::ios::out | std::ios::app);
#else
// never opened
std::ofstream logfile;
#endif

print_logger<logtype::normal>& pl()
{
	static print_logger<logtype::normal> pl(logfile);
	return pl;
}

print_logger<logtype::verbose>& plverb()
{
	static print_logger<logtype::verbose> pl(logfile);
	return pl;
}

print_logger<logtype::fileonly>& plfile()
{
	static print_logger<logtype::fileonly> pl(logfile);
	return pl;
}
