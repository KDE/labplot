# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.9)

project(Vector_BLF
    VERSION 2.4.2
    DESCRIPTION "Vector Binary Log File support library")

# source code documentation
option(OPTION_RUN_DOXYGEN "Run Doxygen" OFF)

# static code analysis
option(OPTION_RUN_CCCC "Run CCCC" OFF)
option(OPTION_RUN_CPPCHECK "Run Cppcheck" OFF)

# dynamic tests
option(OPTION_BUILD_EXAMPLES "Build examples" OFF)
option(OPTION_BUILD_TESTS "Build tests" OFF)
option(OPTION_USE_GCOV "Build with gcov to generate coverage data on execution" OFF)
option(OPTION_USE_GPROF "Build with gprof" OFF)
option(OPTION_ADD_LCOV "Add lcov targets to generate HTML coverage report" OFF)
# Turn OFF, if you are using FetchContent to include it to your project
option(FETCH_CONTENT_INCLUSION "Include project with FetchContent_Declare in another project. In this case the headers and the cmake files are not needed, only the library" OFF)

# directories
include(GNUInstallDirs)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
if(OPTION_RUN_DOXYGEN)
    find_package(Doxygen REQUIRED)
    find_package(Graphviz)
    if(WIN32)
        find_package(HTMLHelp REQUIRED)
    endif()
endif()
if(OPTION_RUN_CCCC)
    find_package(CCCC REQUIRED)
endif()
if(OPTION_RUN_CPPCHECK)
    find_package(Cppcheck REQUIRED)
endif()
if(OPTION_BUILD_TESTS)
    enable_testing()
    find_package(Boost 1.55 COMPONENTS system filesystem unit_test_framework REQUIRED)
endif()
if(OPTION_ADD_LCOV)
    find_package(LCOV REQUIRED)
endif()

# package
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "Tobias Lorenz <tobias.lorenz@gmx.net>")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/GPL-3.0-or-later.txt)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
include(CPack)

# install
install(
    FILES
        CHANGELOG.md
        README.md
        LICENSES/GPL-3.0-or-later.txt
    DESTINATION ${CMAKE_INSTALL_DOCDIR})

# sub directories
add_subdirectory(src)
