#include <iostream>
#include <array>
#include <vector>
#include "../lib/common/net.hpp"
#include "../lib/parse/inoutbox.hpp"

#include "parseoptions.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Sup, nerds. Welcome to Mastosync.\n";
	//getgoodstuff(read_url("https://cybre.space/api/v1/instance"));

	std::array<int, 5> nums{4, 100, 2, 300, -5};

	for (auto v : nums)
	{
		std::cout << "The number " << v << " is " << (is_good(v) ? "good" : "bad") << ".\n";
	}

	std::vector<int> args = parse(argc, argv);
	for (auto v : args)
	{
		std::cout << "You gave me the number " << v << ", which is " << (is_good(v) ? "good" : "bad") << ".\n";
	}
}
