#ifndef _FS_HPP_
#define _FS_HPP_

#ifndef __clang__
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#endif