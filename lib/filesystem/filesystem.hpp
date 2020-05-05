#ifndef FS_HPP
#define FS_HPP

#if MSYNC_USE_BOOST && __has_include(<boost/filesystem.hpp>)
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#elif __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error Could not find filesystem library in <filesystem> or <experimental/filesystem>
#endif

#include <string>

inline std::string as_utf8(const fs::path& p)
{
#if MSYNC_USE_BOOST && __APPLE__
	// this should be fine, OSX's paths should be UTF-8, and OSX is the only platform that should have to use Boost
	return p.string();
#else
	return p.u8string();
#endif
}

#endif 
