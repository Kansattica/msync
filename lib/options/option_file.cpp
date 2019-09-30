#include "option_file.hpp"

using std::getline;

void Read(std::map<std::string, std::string>& parsed, std::string& line)
{
	const auto equals = line.find_first_of('=');
	const auto [it, success] = parsed.insert({ line.substr(0, equals), line.substr(equals + 1) });
}

void Write(std::pair<const std::string, std::string>& kvp, std::ofstream& of)
{
	if (!kvp.second.empty()) //don't serialize
		of << kvp.first << '=' << kvp.second << '\n';
}
