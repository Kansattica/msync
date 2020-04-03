target_sources_local(msync
	PRIVATE
        msync.cpp
        new_account.hpp
        new_account.cpp
        version.hpp
)

if (MSVC)
	target_compile_options(msync PRIVATE /utf-8)
endif()
