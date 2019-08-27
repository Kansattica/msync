cmake_minimum_required(VERSION 3.13..3.15)
project (msync VERSION 0.1
	DESCRIPTION "Store and forward messages with a Mastodon-compatible API server."
	LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS off)

if (MSVC)
	add_compile_options(/W3)
else()
	add_compile_options(-W -Wall)
endif()

add_library(parse "")
add_library(sync "")

# see https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
add_subdirectory(lib/parse)
add_subdirectory(lib/sync)

add_executable(msync console/msync.cpp)

target_link_libraries(msync PRIVATE parse)
target_link_libraries(msync PRIVATE sync)
