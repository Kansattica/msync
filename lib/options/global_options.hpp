#include "filesystem.hpp"
#include "user_options.hpp"

struct global_options
{
    public:
        bool verbose;
        int retries;
        fs::path executable_location = get_exe_location();
        fs::path current_working_directory = fs::current_path();

        std::vector<user_options> accounts_to_sync;

    private:
        fs::path get_exe_location();
};