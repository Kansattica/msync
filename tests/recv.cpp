#include <catch2/catch.hpp>

#include "../lib/sync/recv.hpp"

#include "test_helpers.hpp"
#include "../lib/net_interface/net_interface.hpp"

#include <string>

template <typename make_object>
std::string make_json_array(size_t count, make_object func)
{
	std::string toreturn = '[';
	for (size_t i = 0; i < count; i++)
	{
		toreturn.append(func()).append(',');
	}
	toreturn += ']';
	return toreturn;
}



SCENARIO("Recv downloads and writes the correct number of posts.")
{

}