add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(Filesystem)

if (NOT CXX_FILESYSTEM_HAVE_FS OR NOT Filesystem_FOUND)
	# The only time we should wind up in here now is if the host machine actually doesn't have std::filesystem support
	#see https://en.cppreference.com/w/cpp/compiler_support
	message (WARNING "msync requires std::filesystem support (gcc libstdc++ 8, clang libc++ 7, MSVC 2017 15.7, or macos 10.15) and couldn't find it. Will try to continue anyways." )
else ()
	target_link_libraries(filesystem INTERFACE ${CXX_FILESYSTEM_NAMESPACE})
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
