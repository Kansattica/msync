#include "option_file.hpp"

bool Read(std::map<std::string, std::string>& parsed, std::string&& line)
{
	const auto equals = line.find_first_of('=');
	parsed.emplace(line.substr(0, equals), line.substr(equals + 1));
	return false;
}

void Write(std::map<std::string, std::string>&& map, std::ofstream& of)
{
	for (const auto& kvp : map)
		if (!kvp.second.empty()) //don't serialize
			of << kvp.first << '=' << kvp.second << '\n';
}
