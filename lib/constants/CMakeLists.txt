configure_file(constants.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/constants.hpp)

target_include_directories(constants
		INTERFACE
		${CMAKE_CURRENT_LIST_DIR}
)
