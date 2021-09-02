#=============================================================================
# SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(PC_LIBORIGIN liborigin>=3.0.0 QUIET)
endif ()

find_library(LIBORIGIN_LIBRARIES
    NAMES origin
    HINTS ${PC_LIBORIGIN_LIBRARY_DIRS}
)

find_path(LIBORIGIN_INCLUDE_DIR
    NAMES OriginFile.h
    HINTS ${PC_LIBORIGIN_INCLUDE_DIRS}
)

set(LIBORIGIN_VERSION ${PC_LIBORIGIN_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibOrigin
    REQUIRED_VARS
        LIBORIGIN_LIBRARIES
        LIBORIGIN_INCLUDE_DIR
    VERSION_VAR
        LIBORIGIN_VERSION
)

if(LIBORIGIN_FOUND)
    add_library(liborigin UNKNOWN IMPORTED)
    set_target_properties(liborigin PROPERTIES
	    IMPORTED_LOCATION "${LIBORIGIN_LIBRARIES}"
	    INTERFACE_COMPILE_OPTIONS "${PC_LIBORIGIN_CFLAGS}"
	    INTERFACE_INCLUDE_DIRECTORIES "${LIBORIGIN_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(LIBORIGIN_LIBRARIES LIBORIGIN_INCLUDE_DIR LIBORIGIN_VERSION)

include(FeatureSummary)
set_package_properties(LibOrigin PROPERTIES
	DESCRIPTION "A library for reading OriginLab project files"
    URL "http://sourceforge.net/projects/liborigin"
)

