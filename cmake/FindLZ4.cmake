#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LZ4 lz4 QUIET)

find_library(LZ4_LIBRARIES
    NAMES lz4
    HINTS ${PC_LZ4_LIBRARY_DIRS}
)

find_path(LZ4_INCLUDE_DIR
    NAMES lz4.h
    HINTS ${PC_LZ4_INCLUDE_DIRS}
)

set(LZ4_VERSION "${PC_LZ4_VERSION}")

if(NOT LZ4_VERSION)
    if(EXISTS "${LZ4_INCLUDE_DIR}/lz4.h")
        file(READ "${LZ4_INCLUDE_DIR}/lz4.h" LZ4_H_CONTENT)

        string(REGEX MATCH "#define LZ4_VERSION_MAJOR[ ]+[0-9]+" LZ4_VERSION_MAJOR_MATCH ${LZ4_H_CONTENT})
        string(REGEX MATCH "#define LZ4_VERSION_MINOR[ ]+[0-9]+" LZ4_VERSION_MINOR_MATCH ${LZ4_H_CONTENT})
        string(REGEX MATCH "#define LZ4_VERSION_RELEASE[ ]+[0-9]+" LZ4_VERSION_RELEASE_MATCH ${LZ4_H_CONTENT})

        string(REGEX REPLACE ".*_MAJOR[ ]+(.*)" "\\1" LZ4_VERSION_MAJOR ${LZ4_VERSION_MAJOR_MATCH})
        string(REGEX REPLACE ".*_MINOR[ ]+(.*)" "\\1" LZ4_VERSION_MINOR ${LZ4_VERSION_MINOR_MATCH})
        string(REGEX REPLACE ".*_RELEASE[ ]+(.*)" "\\1"  LZ4_VERSION_RELEASE  ${LZ4_VERSION_RELEASE_MATCH})

        set(LZ4_VERSION "${LZ4_VERSION_MAJOR}.${LZ4_VERSION_MINOR}.${LZ4_VERSION_RELEASE}")
    else()
        set(LZ4_VERSION "0.0.0")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZ4
    REQUIRED_VARS
    LZ4_LIBRARIES
        LZ4_INCLUDE_DIR
    VERSION_VAR
        LZ4_VERSION
)

if(LZ4_FOUND AND NOT TARGET lz4::lz4)
    add_library(lz4::lz4 UNKNOWN IMPORTED)
    set_target_properties(lz4::lz4 PROPERTIES
	IMPORTED_LOCATION "${LZ4_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${PC_LZ4_CFLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${LZ4_INCLUDE_DIR}"
    )
else()
	set(LZ4_LIBRARIES "")
endif()

mark_as_advanced(LZ4_LIBRARIES LZ4_INCLUDE_DIR LZ4_VERSION)

include(FeatureSummary)
set_package_properties(LZ4 PROPERTIES
    DESCRIPTION "Lossless compression algorithm"
    URL "https://lz4.github.io/lz4/"
)
