cmake_minimum_required(VERSION 3.12..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon-compatible API server."
	LANGUAGES CXX)

include(cmake/linktimeoptimization.cmake)

include(cmake/packages.cmake)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_EXTENSIONS off)

add_library(parse STATIC "")
add_library(sync STATIC "")
add_library(common STATIC "")

# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
include(cmake/compatibility.cmake)

add_subdirectory(lib/parse)
add_subdirectory(lib/sync)
add_subdirectory(lib/common)

target_link_libraries(sync PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(common PRIVATE ${CPR_LIBRARIES})

add_executable(msync console/msync.cpp)

target_link_libraries(msync PRIVATE common)
target_link_libraries(msync PRIVATE parse)
target_link_libraries(msync PRIVATE sync)
