#include <iostream>

#include <exception>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <string>

#include "newaccount.hpp"
#include "optionparsing/parseoptions.hpp"

user_options& assume_account(user_options* user, const std::string& name);
void print_stringptr(const std::string* toprint);

int main(int argc, const char* argv[])
{
    print_logger<logtype::fileonly> pl;
    print_logger<logtype::normal> plerr;
    pl << "--- msync started ---\n";

    auto parsed = parse(argc, argv, false);

    auto user = options.select_account(parsed.account);
    try
    {
        switch (parsed.selected)
        {
        case mode::newuser:
            make_new_account(parsed.account);
            break;
        case mode::showopt:
            print_stringptr(assume_account(user, parsed.account).get_option(parsed.toset));
            break;
        case mode::showallopt:
            break;
        case mode::config:
            assume_account(user, parsed.account).set_option(parsed.toset, parsed.optionval);
            break;
        case mode::help:
            break;
        default:
            plerr << "[option not implemented]";
        }
    }
    catch (const std::exception& e)
    {
        plerr << "An error occurred: " << e.what();
        plerr << "\nFor account: " << parsed.account;
    }

    plerr << '\n';

    pl << "--- msync finished normally ---\n";
}

user_options& assume_account(user_options* user, const std::string& name)
{
    if (user == nullptr)
        throw msync_exception("Could not find a match [or an unambiguous match].");
    return *user;
}

void print_stringptr(const std::string* toprint)
{
    print_logger<logtype::normal> pl;
    if (toprint == nullptr)
        pl << "[none]";
    else
        pl << *toprint;
}