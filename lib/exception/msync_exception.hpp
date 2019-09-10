#ifndef _MSYNC_EXCEPTION_HPP_
#define _MSYNC_EXCEPTION_HPP_
#include <exception>
#include <string>

class msync_exception : public std::exception
{
public:
    msync_exception(const std::string& what_arg) : what_msg(what_arg) {}
    const char* what() const noexcept
    {
        return what_msg.c_str();
    }

private:
    std::string what_msg;
};
#endif