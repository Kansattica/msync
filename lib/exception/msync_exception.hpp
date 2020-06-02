#ifndef MSYNC_EXCEPTION_HPP
#define MSYNC_EXCEPTION_HPP

#include <exception>
#include <string>

class msync_exception : public std::exception
{
public:
	msync_exception(const char* what_arg) noexcept : noalloc_what_msg(what_arg) {}
	msync_exception(std::string what_arg) noexcept : what_msg(std::move(what_arg)) {}
	const char* what() const noexcept
	{
		if (noalloc_what_msg != nullptr)
			return noalloc_what_msg;
		return what_msg.c_str();
	}

private:
	const char* noalloc_what_msg = nullptr;
	const std::string what_msg;
};
#endif
