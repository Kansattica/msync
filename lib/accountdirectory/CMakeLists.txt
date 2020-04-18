target_sources_local(accountdirectory
	PRIVATE
	account_directory.cpp
	account_directory.hpp
	)
if(MSYNC_USE_XDG_DIRECTORIES OR MSYNC_BASE_DIRECTORY STREQUAL "")
  target_compile_definitions(accountdirectory
    PRIVATE USE_XDG_DIRECTORY_SPEC=1)
else()
  target_compile_definitions(accountdirectory
    PRIVATE MSYNC_BASE_DIRECTORY=\"${MSYNC_BASE_DIRECTORY}\"
    USING_MSYNC_BASE_DIRECTORY=1)
endif()