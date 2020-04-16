target_include_directories(printlog
		INTERFACE
		${CMAKE_CURRENT_LIST_DIR}
)

target_sources_local(printlog
		PRIVATE
		print_logger.cpp
		print_logger.hpp
)