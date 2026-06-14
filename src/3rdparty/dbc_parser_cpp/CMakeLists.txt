cmake_minimum_required(VERSION 3.16)

# Keep this on one line for release checking
project(dbc VERSION 0.5.0 DESCRIPTION "C++ DBC Parser")

# -- PROJECT OPTIONS  -- #
option(DBC_ENABLE_TESTS "Enable Unittests" OFF)  # Disabled: requires Catch2 via FetchContent
option(DBC_TEST_LOCALE_INDEPENDENCE "Used to deterime if the libary is locale agnostic when it comes to converting floats. You need `de_DE.UTF-8` locale installed for this testing." OFF)
option(DBC_GENERATE_DOCS "Use doxygen if installed to generated documentation files" OFF)
option(DBC_GENERATE_SINGLE_HEADER "This will run the generator for the single header file version. Default is OFF since we make a static build. Requires cargo installed." OFF)
# ---------------------- #

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# package
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
include(CPack)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(FastFloat QUIET)
if (NOT ${FastFloat_FOUND})
	# Use local copy from ../fast_float
	add_subdirectory(../fast_float ${CMAKE_CURRENT_BINARY_DIR}/fast_float EXCLUDE_FROM_ALL)
endif()

# add where to find the source files
list(APPEND SOURCE_FILES
	${PROJECT_SOURCE_DIR}/src/utils.cpp
	${PROJECT_SOURCE_DIR}/src/message.cpp
	${PROJECT_SOURCE_DIR}/src/signal.cpp
	${PROJECT_SOURCE_DIR}/src/dbc.cpp
)

list(APPEND HEADER_FILES
  ${PROJECT_SOURCE_DIR}/include/libdbc/dbc.hpp
  ${PROJECT_SOURCE_DIR}/include/libdbc/message.hpp
  ${PROJECT_SOURCE_DIR}/include/libdbc/signal.hpp
  ${PROJECT_SOURCE_DIR}/include/libdbc/utils/utils.hpp
  ${PROJECT_SOURCE_DIR}/include/libdbc/exceptions/error.hpp
)

if(DBC_ENABLE_TESTS)
	include(CTest)
	add_subdirectory(test)
endif()

if(DBC_GENERATE_DOCS)
	add_subdirectory(doc)
endif()

list(APPEND GCC_CLANG_COMPILE_FLAGS
	-Wall -Wextra -Wpedantic
	-Wconversion -Wint-in-bool-context
	-Wmissing-declarations -Wmissing-field-initializers
	-Werror
)


if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_compile_options(/W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# Clang shadow warnings aren't as sensitive as gcc
	add_compile_options(${GCC_CLANG_COMPILE_FLAGS} -Wshadow)
else()
	add_compile_options(${GCC_CLANG_COMPILE_FLAGS})
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} FastFloat::fast_float)
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_sources(${PROJECT_NAME} INTERFACE ${HEADER_FILES})

if(DBC_GENERATE_SINGLE_HEADER)
	add_custom_target(single_header ALL
						WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
						COMMAND ${CMAKE_SOURCE_DIR}/scripts/create_single_header.sh
	)
endif()

## Installation
# install lib
install(TARGETS ${PROJECT_NAME}
		DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/libdbc DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Generate pkg-config file
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Clang tidy
add_custom_target(clang-tidy-check
	clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json -warnings-as-errors=* ${SOURCE_FILES} ${HEADER_FILES}
	DEPENDS ${SOURCE_FILES} ${HEADER_FILES}
)

add_custom_target(clang-tidy-dump
	clang-tidy -checks=-*,clang-analyzer-*,clang-analyzer-cplusplus*,cert-*,cppcoreguidelines-*,portability-*,readability-*,clang-diagnostic-* -dump-config -p ${CMAKE_BINARY_DIR}/compile_commands.json ${SOURCE_FILES} ${HEADER_FILES} > ../.clang-tidy
	DEPENDS ${SOURCE_FILES} ${HEADER_FILES}
)

add_custom_target(clang-tidy-fix
	clang-tidy -fix-notes -p ${CMAKE_BINARY_DIR}/compile_commands.json ${SOURCE_FILES} ${HEADER_FILES}
	DEPENDS ${SOURCE_FILES} ${HEADER_FILES}
)
