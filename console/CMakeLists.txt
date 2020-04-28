target_sources_local(msync
	PRIVATE
		msync.cpp
		new_account.hpp
		new_account.cpp
		version.hpp
)

configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)

# basically, if you compile CPR with LTO enabled, the gnu compiler emits a bunch of spurious ODR warnings for every library that includes CPR.
# this quiets that down.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set_target_properties(msync PROPERTIES LINK_FLAGS "-Wno-odr")
endif()
