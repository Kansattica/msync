target_sources_local(net
	PRIVATE
        net.hpp
        net.cpp
)

target_sources_local(options
        PRIVATE
        options.cpp
        options.hpp
        option_types.hpp
)

target_sources_local(constants
        INTERFACE
        constants.hpp
)

#set_target_properties (constants PROPERTIES LINKER_LANGUAGE CXX)