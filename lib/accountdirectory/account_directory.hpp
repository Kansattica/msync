#ifndef MSYNC_ACCOUNT_DIR
#define MSYNC_ACCOUNT_DIR

#include "../filesystem/filesystem.hpp"

const fs::path& account_directory_path();

//I hate having this sort of thing that's only for testing, but the static version of account_directory_path is what we want every other time.
#ifdef MSYNC_TESTING
fs::path account_directory_path_uncached();
#endif

#endif
