cmake_minimum_required(VERSION 3.12..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon-compatible API server."
	LANGUAGES CXX C)

include(cmake/linktimeoptimization.cmake)

include(cmake/packages.cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS off)

add_library(parse STATIC "")
add_library(sync STATIC "")
add_library(net STATIC "")
add_library(options STATIC "")
add_library(optionparsing STATIC "")
add_library(printlog INTERFACE)
add_library(constants INTERFACE)
add_executable(msync "")

# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
include(cmake/compatibility.cmake)

add_subdirectory(console)
add_subdirectory(console/optionparsing)
add_subdirectory(lib/parse)
add_subdirectory(lib/sync)
add_subdirectory(lib/common)
add_subdirectory(lib/options)
add_subdirectory(lib/constants)
add_subdirectory(lib/printlog)

target_link_libraries(sync PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(net PRIVATE ${CPR_LIBRARIES})

target_link_libraries(printlog INTERFACE options)

target_link_libraries(options PRIVATE constants whereami filesystem)
target_link_libraries(optionparsing PRIVATE frozen::frozen clipp::clipp)

target_link_libraries(msync PRIVATE net parse sync options optionparsing printlog)
