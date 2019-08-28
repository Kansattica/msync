include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_is_supported OUTPUT error)

if( ipo_is_supported AND CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "IPO / LTO enabled")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
	message(STATUS "IPO / LTO not supported: <${error}>")
endif()
