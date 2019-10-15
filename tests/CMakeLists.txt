add_executable(tests "")
target_sources_local(tests PRIVATE main.cpp option_file.cpp test_helpers.hpp test_helpers.cpp parseoptions.cpp user_options.cpp global_options.cpp util.cpp option_enums.cpp queue_lists.cpp queues.cpp outgoing_post.cpp send.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2 options optionparsing constants util filesystem queue printlog postfile sync netinterface whereami)

add_executable(net_tests "")
target_sources_local(net_tests PRIVATE main.cpp test_https.cpp)
target_link_libraries(net_tests PRIVATE Catch2::Catch2 ${CPR_LIBRARIES})