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
	add_subdirectory(${njson_SOURCE_DIR} ${njson_BINARY_DIR})
endif()

message(STATUS "Downloading clipp...")
FetchContent_Declare(
	clipplib
	GIT_REPOSITORY https://github.com/muellan/clipp.git
	GIT_TAG		   2c32b2f1f7cc530b1ec1f62c92f698643bb368db
	GIT_SHALLOW    TRUE
	GIT_PROGRESS   TRUE
	)

FetchContent_GetProperties(clipplib)
	# suppress that weird INSTALL_DIRS message clipp makes.
	if(NOT clipplib_POPULATED)
	function(message)
		if (NOT MESSAGE_QUIET)
			_message(${ARGN})
		endif()
	endfunction()

	set(MESSAGE_QUIET ON)
	FetchContent_Populate(clipplib)
	add_subdirectory(${clipplib_SOURCE_DIR} ${clipplib_BINARY_DIR} EXCLUDE_FROM_ALL)
	unset(MESSAGE_QUIET)
endif()

message(STATUS "Downloading whereami...")
FetchContent_Declare(
	whereamilib
	GIT_REPOSITORY https://github.com/gpakosz/whereami.git
	GIT_TAG		   f3a86fdf17b49c434a16bb4d9e45a135d4cc25f9
	GIT_PROGRESS   FALSE
	)

FetchContent_GetProperties(whereamilib)
if(NOT whereamilib_POPULATED)
	FetchContent_Populate(whereamilib)
	add_library (whereami STATIC ${whereamilib_SOURCE_DIR}/src/whereami.c  ${whereamilib_SOURCE_DIR}/src/whereami.h)
	target_include_directories(whereami PUBLIC ${whereamilib_SOURCE_DIR}/src)
endif()

message(STATUS "Downloading CPR...")
FetchContent_Declare(
	libcpr
	GIT_REPOSITORY 	https://github.com/kansattica/cpr.git
	GIT_TAG			origin/master
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
	message(STATUS "Configuring CPR...")
	FetchContent_Populate(libcpr)
	message(STATUS "BUILD_SHARED_LIBS IS ${BUILD_SHARED_LIBS}")
	add_subdirectory(${libcpr_SOURCE_DIR})
	#message(STATUS "Descending into ${CPR_INCLUDE_DIRS}")
	#include_directories(${CPR_INCLUDE_DIRS})
	message(STATUS "Prepared CPR libraries ${CPR_LIBRARIES}")
	#target_compile_definitions(${CPR_LIBRARIES} PUBLIC "CURL_STATICLIB")
endif()

if (MSYNC_BUILD_TESTS)
	message(STATUS "Downloading catch2...")
	FetchContent_Declare(
		catch2lib
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG		   2c869e17e4803d30b3d5ca5b0d76387b9db97fa5
		)

	FetchContent_GetProperties(catch2lib)
	if(NOT catch2lib_POPULATED)
		FetchContent_Populate(catch2lib)
		add_subdirectory(${catch2lib_SOURCE_DIR} ${catch2lib_BINARY_DIR})
	endif()
endif()
