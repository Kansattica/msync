add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(Filesystem)

if (NOT CXX_FILESYSTEM_HAVE_FS OR NOT Filesystem_FOUND)
	# The only time we should wind up in here now is if the host machine actually doesn't have std::filesystem support
	# I'd like to take this logic out at a future date, once I'm a little more confident that the issues curl was causing with it are fixed.

	#see https://en.cppreference.com/w/cpp/compiler_support
	message (WARNING "msync requires std::filesystem support (gcc libstdc++ 8, clang libc++ 7, MSVC 2017 15.7, or Xcode 11.0) and couldn't find it. Will try to continue anyways." )
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		 # using Visual Studio C++
		if (MSVC_VERSION GREATER_EQUAL 1910)
			message (STATUS "MSVC version is at least 1910 (detected ${MSVC_VERSION}), so we should have std::filesystem support." )
		else()
			message (WARNING "MSVC version is under 1910 (detected ${MSVC_VERSION}), so you might not have the std::filesystem support msync requires." )
		endif()
	else()
		message (STATUS "I'll try to use Boost." )
		set (Boost_USE_STATIC_LIBS TRUE)
		find_package(Boost COMPONENTS filesystem)
		if (BOOST_FOUND)
			target_compile_definitions(filesystem INTERFACE MSYNC_USE_BOOST=1)
			target_link_libraries(filesystem INTERFACE Boost::filesystem)
		else()
			message (WARNING "Boost not found. I'll try to continue without linking anything special.")
		endif()
	endif()
else ()
	target_link_libraries(filesystem INTERFACE ${CXX_FILESYSTEM_NAMESPACE})
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
