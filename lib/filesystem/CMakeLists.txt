add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(Filesystem)

if (NOT CXX_FILESYSTEM_HAVE_FS OR NOT Filesystem_FOUND)
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
		message (WARNING "I'll try linking the GNU STL filesystem library." )
		target_link_libraries(filesystem INTERFACE -lstdc++fs)
	endif()
else ()
	target_link_libraries(filesystem INTERFACE ${CXX_FILESYSTEM_NAMESPACE})
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
