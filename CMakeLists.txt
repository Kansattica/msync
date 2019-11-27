cmake_minimum_required(VERSION 3.12..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon API-compatible server."
	LANGUAGES CXX C)

option(MSYNC_BUILD_TESTS "Download catch2 and build tests with it." ON)

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) #enable interprocedural optimization for all projects
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
add_library(postfile STATIC "")
add_library(postlist STATIC "")
add_library(net STATIC "")
add_library(netinterface INTERFACE)
add_library(filebacked INTERFACE)
add_library(entities INTERFACE)
add_library(constants INTERFACE)
add_library(exception INTERFACE)
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
add_subdirectory(lib/net)
add_subdirectory(lib/postfile)
add_subdirectory(lib/postlist)
add_subdirectory(lib/exception)
add_subdirectory(lib/printlog)
add_subdirectory(lib/util)


target_include_directories(netinterface INTERFACE lib/net_interface)
target_link_libraries(netinterface INTERFACE filesystem)

target_link_libraries(net PRIVATE ${CPR_LIBRARIES} netinterface filesystem)

target_include_directories(filebacked INTERFACE lib/filebacked)
target_link_libraries(filebacked INTERFACE filesystem)

target_include_directories(entities INTERFACE lib/entities)

target_link_libraries(sync PRIVATE entities printlog queue util netinterface postfile postlist constants filesystem options nlohmann_json::nlohmann_json)

target_link_libraries(options PRIVATE whereami printlog exception filebacked constants)
target_link_libraries(options PUBLIC filesystem) 

target_link_libraries(postfile PRIVATE filebacked printlog util)

target_link_libraries(postlist PRIVATE filesystem entities)

target_link_libraries(queue PRIVATE constants printlog options filebacked exception postfile) 

target_link_libraries(optionparsing PRIVATE clipp::clipp printlog options queue postfile)

target_link_libraries(msync PRIVATE options optionparsing printlog ${CPR_LIBRARIES} util nlohmann_json::nlohmann_json exception postfile queue sync net netinterface)

if (MSYNC_BUILD_TESTS)
	add_subdirectory(tests)
endif()
