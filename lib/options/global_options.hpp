#include "filesystem.hpp"
#include "user_options.hpp"

#include <unordered_map>
#include <string>

struct global_options
{
    public:
        bool verbose = false;
        fs::path executable_location = get_exe_location();
        fs::path current_working_directory = fs::current_path();

        std::unordered_map<std::string, user_options> accounts;

    private:
        fs::path get_exe_location();
};