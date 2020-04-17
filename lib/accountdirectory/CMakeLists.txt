target_sources_local(accountdirectory
	PRIVATE
	account_directory.cpp
	account_directory.hpp
	)
if(MSYNC_USE_XDG_DIRECTORIES)
  target_compile_definitions(accountdirectory
    PRIVATE USE_XDG_DIRECTORY_SPEC=1)
endif()