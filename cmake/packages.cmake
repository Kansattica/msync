include(FetchContent)

add_library (filesystem INTERFACE)
target_compile_features(filesystem INTERFACE cxx_std_17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message (STATUS "Detected clang, using -lc++fs flag for std::filesystem support." )
  target_link_libraries(filesystem INTERFACE -lc++fs )
  #target_link_libraries(filesystem INTERFACE -lc++abi )
  #target_compile_options(filesystem INTERFACE -stdlib=libc++)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message (STATUS "Detected gcc, using -lstdc++fs flag for std::filesystem support." )
  target_link_libraries(filesystem INTERFACE -lstdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND MSVC_VERSION GREATER_EQUAL 1910)
  # using Visual Studio C++
	message (STATUS "MSVC version is at least 1910 (detected ${MSVC_VERSION}), so we should have std::filesystem support." )
else()
	#see https://en.cppreference.com/w/cpp/compiler_support
	message (WARNING "msync requires std::filesystem support (gcc 8+, clang 7+, or MSVC 2017+). Will try to continue anyways." )
endif()


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

message(STATUS "Downloading serge-sans-paille/frozen...")
FetchContent_Declare(
	frozenlib
	GIT_REPOSITORY https://github.com/serge-sans-paille/frozen.git
	GIT_TAG		   origin/master
	GIT_SHALLOW    TRUE
	GIT_PROGRESS   TRUE
	)

set (frozen.installation OFF CACHE BOOL "Don't install frozen, just link it in." FORCE)
FetchContent_GetProperties(frozenlib)
if(NOT frozenlib_POPULATED)
	FetchContent_Populate(frozenlib)
	add_subdirectory(${frozenlib_SOURCE_DIR} ${frozenlib_BINARY_DIR})
endif()

message(STATUS "Downloading TCLAP...")
FetchContent_Declare(
	tclaplib
	GIT_REPOSITORY https://git.code.sf.net/p/tclap/code
	GIT_TAG		   v1.2.2
	GIT_SHALLOW    TRUE
	GIT_PROGRESS   TRUE
	)

FetchContent_GetProperties(tclaplib)
if(NOT tclaplib_POPULATED)
	FetchContent_Populate(tclaplib)
	set (TCLAP_LIBRARIES "${tclaplib_SOURCE_DIR}/include")
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