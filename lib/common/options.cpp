#include <string>
#include <vector>

struct global_options {
    bool verbose;
};

struct user_options {
    std::string account_name;
    std::vector<std::string> favorites;


};