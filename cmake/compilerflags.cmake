if (MSVC)
	add_compile_options("/utf-8")
else()
	add_compile_options(-Wall -Wextra -pedantic)
endif()

# basically, if you compile CPR with LTO enabled, the gnu compiler emits a bunch of spurious ODR warnings for every library that includes CPR.
# this quiets that down.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set_target_properties(msync PROPERTIES LINK_FLAGS "-Wno-odr")
endif()
