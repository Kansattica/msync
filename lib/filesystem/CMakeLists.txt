add_library(filesystem INTERFACE)

target_compile_features(filesystem INTERFACE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

find_package(Filesystem REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # use GNU STL because we're on a Linux system
  # later, bother detecting if we should use the llvm or gnu stl
  message (STATUS "Detected clang, linking GNU STL for std::filesystem support." )
  target_link_libraries(filesystem INTERFACE -lstdc++fs)
  #target_link_libraries(filesystem INTERFACE c++fs )
  #target_link_libraries(filesystem INTERFACE c++abi )
  #target_compile_options(filesystem INTERFACE -stdlib=libc++)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message (STATUS "Detected gcc, linking GNU STL for std::filesystem support." )
  target_link_libraries(filesystem INTERFACE -lstdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND MSVC_VERSION GREATER_EQUAL 1910)
  # using Visual Studio C++
	message (STATUS "MSVC version is at least 1910 (detected ${MSVC_VERSION}), so we should have std::filesystem support." )
else()
	#see https://en.cppreference.com/w/cpp/compiler_support
	message (WARNING "msync requires std::filesystem support (gcc 8+, clang 7+, or MSVC 2017+). Will try to continue anyways." )
endif()

target_include_directories(filesystem
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
