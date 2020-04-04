add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(Filesystem)

if (NOT CXX_FILESYSTEM_HAVE_FS OR NOT Filesystem_FOUND)
	# The only time we should wind up in here now is if the host machine actually doesn't have std::filesystem support

	message (STATUS "No working std::filesystem implementation found. I'll try to use Boost." )
	set (Boost_USE_STATIC_LIBS TRUE)
	set (Boost_USE_MULTITHREADED FALSE)
	find_package(Boost COMPONENTS filesystem)
	if (BOOST_FOUND)
		target_compile_definitions(filesystem INTERFACE MSYNC_USE_BOOST=1)
		target_link_libraries(filesystem INTERFACE Boost::filesystem)
	else()
		message (WARNING "Boost not found. msync requires std::filesystem support (gcc libstdc++ 8, clang libc++ 7, MSVC 2017 15.7, or Xcode 11.0) or boost::filesystem and couldn't find either. Will try to continue anyways.")
	endif()
else ()
	target_link_libraries(filesystem INTERFACE ${CXX_FILESYSTEM_NAMESPACE})
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
