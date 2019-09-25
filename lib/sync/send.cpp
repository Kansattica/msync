#include "send.hpp"

#include "../options/global_options.hpp"
#include <print_logger.hpp>

void send_all(int retries)
{
	print_logger pl;
	for (auto& user : options.accounts)
	{
		pl << "Sending queued information for " << user.first << '\n';
		pl.flush(); // make sure the file looks right
		send(user.second, retries);
	}

}

void send(const user_options& account, int retries)
{

}