add_executable(tests "")
target_sources_local(tests PRIVATE main.cpp option_file.cpp test_helpers.hpp test_helpers.cpp user_options.cpp global_options.cpp util.cpp option_enums.cpp queue_list.cpp queues.cpp send.cpp recv.cpp read_response.cpp outgoing_post.cpp parse_options.cpp post_list.cpp mock_network.hpp executable_location.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2 options optionparsing constants util filesystem queue printlog postfile sync netinterface executablelocation postlist entities exception)


add_executable(net_tests "")
target_sources_local(net_tests PRIVATE main.cpp https_and_gzip.cpp)
target_link_libraries(net_tests PRIVATE Catch2::Catch2 ${CPR_LIBRARIES})

# suppress all the warnings MSVC generates when from Catch2's macros
if(MSVC)
	target_compile_options(tests PRIVATE /wd6319 /wd6237)
	target_compile_options(net_tests PRIVATE /wd6319 /wd6237)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set_target_properties(net_tests PROPERTIES LINK_FLAGS "-Wno-odr")
endif()