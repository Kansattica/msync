include(FetchContent)

message(STATUS "Downloading nlohmann json...")
FetchContent_Declare(
	njson
	URL 		https://github.com/nlohmann/json/archive/v3.7.0.zip
	URL_HASH 	SHA512=dc39328ba58806dc92234becf14ee3ca3279d4fad33c1cfa12d63ded94b859d98bc21d4fd247eaf68fca8a03a4b7de771e6673c7ecd52803678e584b7d8901ab
	)

option(JSON_BuildTests "" OFF)
#FetchContent_MakeAvailable(json) Not available in cmake 13
FetchContent_GetProperties(njson)
if(NOT njson_POPULATED)
	FetchContent_Populate(njson)
	message(STATUS "Building nlohmann json...")
	add_subdirectory(${njson_SOURCE_DIR} ${njson_BINARY_DIR})
endif()

FetchContent_Declare(
	libcpr
	GIT_REPOSITORY 	https://github.com/kansattica/cpr.git
	GIT_TAG			master
	GIT_SHALLOW		TRUE 	
)
option(USE_SYSTEM_CURL "" ON)
option(BUILD_CPR_TESTS "" OFF)
set (BUILD_TESTING OFF CACHE BOOL "If you must build curl from source, don't build the tests." FORCE)
set (BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libcurl and cpr." FORCE)
add_definitions(-DCURL_STATICLIB)
if (MSVC)
	set (CMAKE_USE_WINSSL ON CACHE BOOL "Use winssl" FORCE)
	set (CMAKE_USE_OPENSSL OFF CACHE BOOL "Don't use openssl" FORCE)
endif()
FetchContent_GetProperties(libcpr)
if(NOT libcpr_POPULATED)
	FetchContent_Populate(libcpr)
	message(STATUS "BUILD_SHARED_LIBS IS ${BUILD_SHARED_LIBS}")
	add_subdirectory(${libcpr_SOURCE_DIR})
	#message(STATUS "Descending into ${CPR_INCLUDE_DIRS}")
	#include_directories(${CPR_INCLUDE_DIRS})
	message(STATUS "Prepared CPR libraries ${CPR_LIBRARIES}")
	#target_compile_definitions(${CPR_LIBRARIES} PUBLIC "CURL_STATICLIB")
endif()
