target_sources_local(msync
	PRIVATE
		msync.cpp
		new_account.hpp
		new_account.cpp
		version.hpp
)

configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
