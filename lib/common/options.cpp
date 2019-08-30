#include "options.hpp"

#include <filesystem>

global_options options;

void set_locations(const char *executable_location)
{
    options.current_working_directory = std::filesystem::current_path();
    options.executable_location = std::filesystem::path(executable_location);
    options.executable_location.remove_filename();
}

std::optional<std::string> account_exists(std::string name)
{
    return {};
}