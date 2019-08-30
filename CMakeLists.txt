cmake_minimum_required(VERSION 3.12..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon-compatible API server."
	LANGUAGES CXX)

include(cmake/linktimeoptimization.cmake)

include(cmake/packages.cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS off)

add_library(parse STATIC "")
add_library(sync STATIC "")
add_library(net STATIC "")
add_library(options STATIC "")
add_library(constants INTERFACE)

# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
include(cmake/compatibility.cmake)

add_subdirectory(lib/parse)
add_subdirectory(lib/sync)
add_subdirectory(lib/common)

target_link_libraries(sync PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(parse PRIVATE frozen::frozen)
target_link_libraries(net PRIVATE ${CPR_LIBRARIES})

target_link_libraries(options PRIVATE constants)

add_executable(msync console/msync.cpp console/parseoptions.cpp console/parseoptions.hpp)

target_include_directories(msync PRIVATE ${TCLAP_LIBRARIES})

target_link_libraries(msync PRIVATE net)
target_link_libraries(msync PRIVATE parse)
target_link_libraries(msync PRIVATE sync)
target_link_libraries(msync PRIVATE options)
