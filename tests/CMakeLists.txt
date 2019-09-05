add_executable(tests "")
target_sources_local(tests PRIVATE main.cpp optionfile.cpp argparsing.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2 options)