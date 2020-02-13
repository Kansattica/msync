#include "option_file.hpp"

bool Read(std::map<std::string, std::string, std::less<>>& parsed, std::string&& line)
{
	const auto equals = line.find_first_of('=');
	parsed.emplace(line.substr(0, equals), line.substr(equals + 1));
	return false;
}


void Write(std::map<std::string, std::string, std::less<>>&& map, std::ofstream& of)
{
	using namespace std::string_literals;
	map.try_emplace("file_version"s, "1"s);

	for (const auto& kvp : map)
		if (!kvp.second.empty()) //don't serialize
			of << kvp.first << '=' << kvp.second << '\n';
}
