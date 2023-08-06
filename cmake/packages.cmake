include(FetchContent)

if(MSYNC_NLOHMANN_JSON_DIR STREQUAL "")
	message(STATUS "Downloading Nlohmann JSON...")
	FetchContent_Declare(
		njson
		URL https://github.com/nlohmann/json/releases/download/v3.10.5/include.zip
		URL_HASH SHA256=b94997df68856753b72f0d7a3703b7d484d4745c567f3584ef97c96c25a5798e
		)

	#FetchContent_MakeAvailable(json) Not available in cmake 13
	FetchContent_GetProperties(njson)
	if(NOT njson_POPULATED)
		FetchContent_Populate(njson)
		add_library(nlohmannjson INTERFACE)
		target_include_directories(nlohmannjson INTERFACE ${njson_SOURCE_DIR}/single_include)
	endif()
else()
	message(STATUS "Looking for Nlohmann JSON in ${MSYNC_NLOHMANN_JSON_DIR}.")
	add_library(nlohmannjson INTERFACE)
	target_include_directories(nlohmannjson INTERFACE MSYNC_NLOHMANN_JSON_DIR)
endif()

if(MSYNC_CLIPP_DIR STREQUAL "")
	message(STATUS "Downloading clipp...")
	FetchContent_Declare(
		clipplib
		GIT_REPOSITORY https://github.com/muellan/clipp.git
		GIT_TAG 	2c32b2f1f7cc530b1ec1f62c92f698643bb368db
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
else()
	message(STATUS "Looking for clipp in ${MSYNC_CLIPP_DIR}.")
	add_subdirectory(${clipplib_CLIPP_DIR} EXCLUDE_FROM_ALL)
endif()

if (NOT MSYNC_USER_CONFIG)
	if(MSYNC_WHEREAMI_DIR STREQUAL "")
		message(STATUS "Downloading whereami...")
		FetchContent_Declare(
			whereamilib
			GIT_REPOSITORY https://github.com/gpakosz/whereami.git
			GIT_TAG	       6a8536a8b2d8c1903f22333c1a130a142f6d31de
			)

		FetchContent_GetProperties(whereamilib)
		if(NOT whereamilib_POPULATED)
			FetchContent_Populate(whereamilib)
			add_library (whereami STATIC ${whereamilib_SOURCE_DIR}/src/whereami.c  ${whereamilib_SOURCE_DIR}/src/whereami.h)
			target_include_directories(whereami PUBLIC ${whereamilib_SOURCE_DIR}/src)
		endif()
	else()
		message(STATUS "Looking for whereami in ${MSYNC_WHEREAMI_DIR}.")
		add_library (whereami STATIC ${MSYNC_WHEREAMI_DIR}/whereami.c  ${MSYNC_WHEREAMI_DIR}/whereami.h)
		target_include_directories(whereami PUBLIC ${MSYNC_WHEREAMI_DIR}/src)
	endif()
else()
		add_library(whereami INTERFACE)
endif()

if(MSYNC_CPR_DIR STREQUAL "")
message(STATUS "Downloading CPR...")
FetchContent_Declare(
	libcpr
	GIT_REPOSITORY 	https://github.com/kansattica/cpr.git
	GIT_TAG			20d438db91d5be7acb3c2ba0b3183f8872287b58
)
option(USE_SYSTEM_CURL "Try to use the system's libcurl instead of downloading and statically linking." ON)
option(MSYNC_DOWNLOAD_ZLIB "If downloading and building curl on Windows, try to download zlib as well." ON)
option(BUILD_CPR_TESTS "" OFF)
set (BUILD_TESTING OFF CACHE BOOL "If you must build curl from source, don't build the tests." FORCE)
set (BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libcurl and cpr." FORCE)

if (ipo_is_supported)
	set (CURL_LTO ON CACHE BOOL "Turn on link time optimization for curl" FORCE)
endif()

if(USE_SYSTEM_CURL)
	   find_package(CURL)
endif()

if (NOT USE_SYSTEM_CURL OR NOT CURL_FOUND)
	# Okay, I lost a lot of time on this, so:
	# - if you wind up building curl from source, curl's cmake build does something that... captures or messes up the policy settings
	# that make it so that FindFilesystem later on doesn't use the right policies (CMP0056, 66 and 67). Including it here fixes that.
	include(CheckCXXSourceCompiles)

	set (USE_SYSTEM_CURL OFF CACHE BOOL "Don't use system curl if we don't have it." FORCE)
	set (ENABLE_MANUAL OFF CACHE BOOL "Tell curl not to bother trying to make the manual." FORCE)

	if (MSVC AND MSYNC_DOWNLOAD_ZLIB)
		set(CMAKE_POLICY_DEFAULT_CMP0074 NEW) #force curl to honor ZLIB_ROOT
		set(ZLIB_DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/depends/zlib)
		set(ZLIB_HEADER_DIR ${CMAKE_SOURCE_DIR}/external/zlib/include)
		file(MAKE_DIRECTORY ${ZLIB_DOWNLOAD_DIR})

		file(COPY ${ZLIB_HEADER_DIR}
			DESTINATION
			${ZLIB_DOWNLOAD_DIR})

		# did you know that the only way I could get up-to-date static zlib was compiling it myself?
		if (CMAKE_SIZEOF_VOID_P MATCHES "8")
			message(STATUS "Downloading 64-bit zlib...")
			file(DOWNLOAD "https://kansattica.github.io/msync_deps/zlib1.2.11-winx64.lib"
				${ZLIB_DOWNLOAD_DIR}/lib/zlibstatic.lib
				EXPECTED_HASH SHA256=d65fe524750d8f6001c5b3f0a3cbac56c17f27bf5ff1f86d1bc9e20ae1d5abc7)
		else()
			message(STATUS "Downloading 32-bit zlib...")
			file(DOWNLOAD "https://kansattica.github.io/msync_deps/zlib1.2.11-winx86.lib"
				${ZLIB_DOWNLOAD_DIR}/lib/zlibstatic.lib
				EXPECTED_HASH SHA256=f28f0bed6ec9868e14a3f0a62e53fd9d15473a35d832194c0f12cd6d5c284f34)
		endif()

		set(ZLIB_ROOT ${ZLIB_DOWNLOAD_DIR})
		find_package(ZLIB)
	endif()
endif()

#unset these because CPR's build will run its own find_package(CURL)
UNSET(CURL_FOUND)
UNSET(CURL_INCLUDE_DIRS)
UNSET(CURL_LIBRARIES)
UNSET(CURL_VERSION_STRING)

if (NOT USE_SYSTEM_CURL AND UNIX)
	# you can try to statically link openssl, but you'll run into problems with the cert store
	#set (OPENSSL_USE_STATIC_LIBS TRUE)
	set (OPENSSL_USE_STATIC_LIBS FALSE)

	# static openssl has problems with finding the correct cert store
	# someday, we'll be able to handle this in application code, but not today
	# if you're building this yourself, I suggest either dynamically linking your system's libcurl with openssl 
	# (install from your package mangager if you can) or turning off OPENSSL_USE_STATIC_LIBS because the libraries that come with your system are 
	# more likely to know where the certs are
	# this is for portability more than anything
	set(CURL_CA_FALLBACK ON CACHE BOOL
			"Set ON to use built-in CA store of TLS backend. Defaults to OFF")
endif()
# add_definitions(-DCURL_STATICLIB)
if (MSVC)
	# it's called CMAKE_USE_SCHANNEL now
	#set (CMAKE_USE_WINSSL ON CACHE BOOL "Use winssl" FORCE)
	set (CMAKE_USE_SCHANNEL ON CACHE BOOL "enable Windows native SSL/TLS" FORCE)
	set (CMAKE_USE_OPENSSL OFF CACHE BOOL "Don't use openssl" FORCE)
endif()

FetchContent_GetProperties(libcpr)
if(NOT libcpr_POPULATED)
	message(STATUS "Configuring CPR...")
	FetchContent_Populate(libcpr)
	message(STATUS "BUILD_SHARED_LIBS IS ${BUILD_SHARED_LIBS}")
	add_subdirectory(${libcpr_SOURCE_DIR} ${libcpr_BINARY_DIR} EXCLUDE_FROM_ALL)
	#message(STATUS "Descending into ${CPR_INCLUDE_DIRS}")
	#include_directories(${CPR_INCLUDE_DIRS})
	message(STATUS "Prepared CPR libraries ${CPR_LIBRARIES}")
	#target_compile_definitions(${CPR_LIBRARIES} PUBLIC "CURL_STATICLIB")
endif()
else()
	message(STATUS "Looking for CPR in ${MSYNC_CPR_DIR}.")
	add_subdirectory(${MSYNC_CPR_DIR} EXCLUDE_FROM_ALL)

endif()

if (MSYNC_BUILD_TESTS)
	if(MSYNC_CATCH2_DIR STREQUAL "")
		message(STATUS "Downloading catch2...")
		FetchContent_Declare(
			catch2lib
			GIT_REPOSITORY	https://github.com/catchorg/Catch2.git
			GIT_TAG 		v2.13.8
			GIT_SHALLOW		TRUE
			)

		FetchContent_GetProperties(catch2lib)
		if(NOT catch2lib_POPULATED)
			FetchContent_Populate(catch2lib)
			add_subdirectory(${catch2lib_SOURCE_DIR} ${catch2lib_BINARY_DIR} EXCLUDE_FROM_ALL)
		endif()
	else()
		message(STATUS "Looking for catch2 in ${MSYNC_CATCH2_DIR}.");
		add_subdirectory(${MSYNC_CATCH2_DIR} EXCLUDE_FROM_ALL)
	endif()
endif()
