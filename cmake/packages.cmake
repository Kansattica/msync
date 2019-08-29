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


if (MSVC)
	message(STATUS "Looking to see if we have curl installed...")
	find_package(CURL)

	if (NOT CURL_FOUND)
		message(STATUS "Couldn't find an installed cURL library. Downloading a source build.")
		FetchContent_Declare(
			curllib
			GIT_REPOSITORY 	https://github.com/curl/curl.git
			GIT_TAG 		curl-7_65_3
			GIT_SHALLOW 	TRUE
			GIT_PROGRESS 	TRUE
			)
		if (NOT curllib_POPULATED)
			FetchContent_Populate(curllib)
			message (STATUS "Building curl in ${curllib_BINARY_DIR}...")
			execute_process(
				COMMAND "cmake" "${curllib_SOURCE_DIR}" "-DCMAKE_USE_WINSSL=ON" "-DHTTP_ONLY=ON" "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DENABLE_DEBUG=OFF" "-DCMAKE_BUILD_TYPE=Release"
				WORKING_DIRECTORY ${curllib_BINARY_DIR}
				)
			execute_process(
				COMMAND "cmake" "--build" "." "--config" "Release" "--clean-first"
				WORKING_DIRECTORY ${curllib_BINARY_DIR}
				)

			#add_subdirectory(${curllib_SOURCE_DIR})
			SET (CURL_LIBRARY ${curllib_BINARY_DIR}/lib/Release/libcurl.lib)
			SET (CURL_INCLUDE_DIR ${curllib_BINARY_DIR}/include)
		endif()
	endif()
	add_definitions(-DCURL_STATICLIB)
	#unset these because the cpr build will run find_package(curl) again to find the binaries we just got
	unset(CURL_FOUND)
	unset(CURL_INCLUDE_DIRS)
	unset(CURL_LIBRARIES)
	unset(CURL_VERSION_STRING)
endif()

FetchContent_Declare(
	libcpr
	GIT_REPOSITORY 	https://github.com/kansattica/cpr.git
	GIT_TAG			master
	GIT_SHALLOW		TRUE 	
)
option(USE_SYSTEM_CURL "" ON)
option(BUILD_CPR_TESTS "" OFF)
FetchContent_GetProperties(libcpr)
if(NOT libcpr_POPULATED)
	FetchContent_Populate(libcpr)
	add_subdirectory(${libcpr_SOURCE_DIR})
	#message(STATUS "Descending into ${CPR_INCLUDE_DIRS}")
	#include_directories(${CPR_INCLUDE_DIRS})
	message(STATUS "Prepared CPR libraries ${CPR_LIBRARIES}")
	#target_compile_definitions(${CPR_LIBRARIES} PUBLIC "CURL_STATICLIB")
endif()
