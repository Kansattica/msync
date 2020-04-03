#ifndef _FS_HPP_
#define _FS_HPP_

#ifdef MSYNC_USE_BOOST
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

#endif 
