cmake_minimum_required(VERSION 3.12..3.15)

project (msync VERSION 0.9.9.5
	DESCRIPTION "Store and forward messages with a Mastodon API-compatible server."
	HOMEPAGE_URL "https://github.com/Kansattica/msync"
	LANGUAGES CXX C)
set (CPACK_PACKAGE_CONTACT "grace@hypnovir.us")

option(MSYNC_BUILD_TESTS "Download catch2 and build tests with it." ON)
option(MSYNC_FILE_LOG "Log debug messages to msync.log" ON)
option(MSYNC_USER_CONFIG "Store configuration in the OS user configuration folder. Otherwise, store configuration in msync_accounts in the executable's directory." OFF)

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) #enable interprocedural optimization for all projects

include(cmake/linktimeoptimization.cmake)

include(cmake/packages.cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include(cmake/compilerflags.cmake)

add_library(queue STATIC "")
add_library(sync STATIC "")
add_library(options STATIC "")
add_library(util STATIC "")
add_library(optionparsing STATIC "")
add_library(printlog STATIC "")
add_library(postfile STATIC "")
add_library(postlist STATIC "")
add_library(net STATIC "")
add_library(accountdirectory STATIC "")
add_library(fixlocale STATIC "")
add_library(netinterface INTERFACE)
add_library(filebacked INTERFACE)
add_library(entities INTERFACE)
add_library(constants INTERFACE)
add_library(exception INTERFACE)
add_executable(msync "")


# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
include(cmake/compatibility.cmake)

add_subdirectory(lib/queue)
add_subdirectory(lib/sync)
add_subdirectory(lib/filesystem)
add_subdirectory(lib/options)
add_subdirectory(lib/constants)
add_subdirectory(lib/net)
add_subdirectory(lib/accountdirectory)
add_subdirectory(lib/fixlocale)
add_subdirectory(lib/postfile)
add_subdirectory(lib/postlist)
add_subdirectory(lib/exception)
add_subdirectory(lib/printlog)
add_subdirectory(lib/util)
add_subdirectory(console/optionparsing)
add_subdirectory(console)

target_include_directories(netinterface INTERFACE lib/netinterface)
target_link_libraries(netinterface INTERFACE filesystem)

target_link_libraries(net PRIVATE ${CPR_LIBRARIES} netinterface filesystem)

target_include_directories(filebacked INTERFACE lib/filebacked)
target_link_libraries(filebacked INTERFACE filesystem)

target_include_directories(entities INTERFACE lib/entities)

target_link_libraries(sync PRIVATE entities printlog queue util netinterface postfile postlist constants filesystem options nlohmannjson)

target_link_libraries(accountdirectory PRIVATE whereami filesystem constants)

target_link_libraries(options PRIVATE printlog exception filebacked constants accountdirectory)
target_link_libraries(options PUBLIC filesystem) 

target_link_libraries(postfile PRIVATE filebacked printlog util)

target_link_libraries(postlist PRIVATE filesystem entities)

target_link_libraries(printlog PRIVATE constants)

target_link_libraries(queue PRIVATE constants printlog filebacked exception postfile util) 

target_link_libraries(optionparsing PRIVATE clipp::clipp printlog options queue postfile)

target_link_libraries(msync PRIVATE options optionparsing printlog ${CPR_LIBRARIES} util nlohmannjson exception postfile queue sync net netinterface accountdirectory
	fixlocale)


if (MSYNC_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

install(TARGETS msync
	RUNTIME DESTINATION bin
)

if (UNIX)
	set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc1, libcurl4")
	set(CPACK_GENERATOR "DEB")
endif()
include(CPack)
