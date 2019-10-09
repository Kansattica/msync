#include "option_file.hpp"

using std::getline;

void Read(std::map<std::string, std::string>& parsed, std::string&& line)
{
	const auto equals = line.find_first_of('=');
	parsed.emplace(std::make_pair(line.substr(0, equals), line.substr(equals + 1)));
}

void Write(std::map<std::string, std::string>&& map, std::ofstream& of)
{
	for (auto& kvp : map)
		if (!kvp.second.empty()) //don't serialize
			of << kvp.first << '=' << kvp.second << '\n';
}
