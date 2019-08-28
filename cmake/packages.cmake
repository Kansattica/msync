include(FetchContent)

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


if (MSVC)
	option(USE_SYSTEM_CURL "" ON)
	if (CMAKE_GENERATOR_PLATFORM MATCHES x64)
		FetchContent_Declare(
			curllib
			URL 		https://curl.haxx.se/windows/dl-7.65.3_1/curl-7.65.3_1-win64-mingw.zip
			URL_HASH 	SHA256=19384567361d89261b92373ef4950e257c2fb4c72a7c4fbc4bbdaf463f83ff39
			)
	else()
		FetchContent_Declare(
			curllib
			URL 		https://curl.haxx.se/windows/dl-7.65.3_1/curl-7.65.3_1-win32-mingw.zip
			URL_HASH 	SHA256=031f66560ab8eb324cba331dd381cfa5ba73edb92da6047bedad20cb3736d0e8
			)
	endif()
	FetchContent_Populate(curllib)
	SET (CURL_LIBRARY ${curllib_SOURCE_DIR}/lib/libcurl.dll.a)
	SET (CURL_INCLUDE_DIR ${curllib_SOURCE_DIR}/include)
endif()

FetchContent_Declare(
	libcpr
	GIT_REPOSITORY 	https://github.com/kansattica/cpr.git
	GIT_TAG			master
	GIT_SHALLOW		TRUE 	
)
option(BUILD_CPR_TESTS "" OFF)
FetchContent_GetProperties(libcpr)
if(NOT libcpr_POPULATED)
	FetchContent_Populate(libcpr)
	add_subdirectory(${libcpr_SOURCE_DIR})
	include_directories(${CPR_INCLUDE_DIRS})
endif()