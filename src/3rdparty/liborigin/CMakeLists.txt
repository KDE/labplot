# library version
project( liborigin
	VERSION 3.0.1
	LANGUAGES CXX
)

# compile-time configuration variables to be linked in
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# source files
set (sources
	OriginFile.cpp
	OriginParser.cpp
	OriginAnyParser.cpp
)

# header files for development
set (devel-headers
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	OriginObj.h
	OriginFile.h
	OriginParser.h
	tree.hh
)

# static library
add_library (liborigin-static STATIC ${sources})
set_target_properties(liborigin-static PROPERTIES OUTPUT_NAME "origin" POSITION_INDEPENDENT_CODE ON)

# inform on log setting
if (ENABLE_ORIGIN_PARSER_LOG)
	target_compile_definitions(liborigin-static PRIVATE GENERATE_CODE_FOR_LOG=${ENABLE_ORIGIN_PARSER_LOG})
endif ()
