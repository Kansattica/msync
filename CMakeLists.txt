cmake_minimum_required(VERSION 3.12..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon-compatible API server."
	LANGUAGES CXX C)

option(MSYNC_BUILD_TESTS "Download catch2 and build tests with it." ON)

include(cmake/linktimeoptimization.cmake)

include(cmake/packages.cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS off)

add_library(queue STATIC "")
add_library(sync STATIC "")
add_library(options STATIC "")
add_library(util STATIC "")
add_library(optionparsing STATIC "")
add_library(printlog STATIC "")
add_library(constants INTERFACE)
add_library(exception INTERFACE)
add_library(filesystem INTERFACE)
add_executable(msync "")

# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
include(cmake/compatibility.cmake)

add_subdirectory(console)
add_subdirectory(console/optionparsing)
add_subdirectory(lib/queue)
add_subdirectory(lib/sync)
add_subdirectory(lib/filesystem)
add_subdirectory(lib/options)
add_subdirectory(lib/constants)
add_subdirectory(lib/exception)
add_subdirectory(lib/printlog)
add_subdirectory(lib/util)

target_link_libraries(sync PRIVATE nlohmann_json::nlohmann_json cpr options printlog queue util)

target_link_libraries(options PRIVATE constants whereami printlog exception )
target_link_libraries(options PUBLIC filesystem) 

target_link_libraries(queue PUBLIC filesystem) 
target_link_libraries(queue PRIVATE constants printlog options) 

target_link_libraries(optionparsing PRIVATE clipp::clipp printlog options queue util)

target_link_libraries(msync PRIVATE options optionparsing printlog ${CPR_LIBRARIES} util nlohmann_json::nlohmann_json exception)

if (MSYNC_BUILD_TESTS)
	add_subdirectory(tests)
endif()