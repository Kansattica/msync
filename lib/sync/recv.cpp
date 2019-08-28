#include "recv.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void getgoodstuff()
{
	json jerry = {
		{"cool", "beans"},
		{"counting", 69},
		{"hey there", {"friend", true}}
	};
}
