#include "filesystem.hpp"
#include "user_options.hpp"

struct global_options
{
    public:
        bool verbose = false;
        int retries = 3;
        fs::path executable_location = get_exe_location();
        fs::path current_working_directory = fs::current_path();

        std::vector<user_options> accounts;

    private:
        fs::path get_exe_location();
};