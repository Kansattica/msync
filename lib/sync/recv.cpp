#include "recv.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>

using json = nlohmann::json;

void getgoodstuff()
{
	json jerry = {
		{"cool", "beans"},
		{"counting", 69},
		{"hey there", {"friend", true}}};

	std::cout << std::setw(4) << jerry << std::endl;
}