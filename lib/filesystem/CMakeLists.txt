add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

# for some reason, this find_filesystem script doesn't think MSVC has filesystem support when it does.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
    if (MSVC_VERSION GREATER_EQUAL 1910)
		message (STATUS "MSVC version is at least 1910 (detected ${MSVC_VERSION}), so we should have std::filesystem support." )
    else()
		message (WARNING "MSVC version is under 1910 (detected ${MSVC_VERSION}), so you might not have the std::filesystem support msync requires." )
    endif()
else()
	find_package(Filesystem)

    if (NOT CXX_FILESYSTEM_HAVE_FS OR NOT Filesystem_FOUND)
		#see https://en.cppreference.com/w/cpp/compiler_support
		message (WARNING "msync requires std::filesystem support (gcc 8+, clang 7+, or MSVC 2017+) and couldn't find it. Will try to continue anyways." )
		target_link_libraries(filesystem INTERFACE -lstdc++fs)
    else ()
        target_link_libraries(filesystem INTERFACE ${CXX_FILESYSTEM_NAMESPACE})
	endif()
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
