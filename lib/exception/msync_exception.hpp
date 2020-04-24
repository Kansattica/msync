#ifndef MSYNC_EXCEPTION_HPP
#define MSYNC_EXCEPTION_HPP

#include <exception>
#include <string>

class msync_exception : public std::exception
{
public:
	msync_exception(std::string what_arg) : what_msg(what_arg) {}
	const char* what() const noexcept
	{
		return what_msg.c_str();
	}

private:
	std::string what_msg;
};
#endif