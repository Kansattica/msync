#include <iostream>
#include "../lib/common/net.hpp"
#include "../lib/sync/recv.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";
	std::cout << read_url() << std::endl;

	getgoodstuff();
}
