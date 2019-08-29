#include "recv.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>

using json = nlohmann::json;

void getgoodstuff(std::string jsonish)
{
	json jerry = json::parse(jsonish);

	std::cout << std::setw(4) << jerry << std::endl;
}