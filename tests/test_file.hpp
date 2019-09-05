#include "../lib/options/filesystem.hpp"

//ensures a file doesn't exist before and after each test run.
struct test_file
{
public:
    test_file(fs::path name) : filename(name)
    {
        if (fs::exists(filename))
            fs::remove(filename);
    };

    ~test_file()
    {
        if (fs::exists(filename))
            fs::remove(filename);
    };

private:
    fs::path filename;
};