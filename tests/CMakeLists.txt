add_executable(tests "")
target_sources_local(tests PRIVATE main.cpp option_file.cpp test_helpers.hpp parseoptions.cpp user_options.cpp global_options.cpp util.cpp option_enums.cpp queue_lists.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2 options optionparsing constants util filesystem parse)