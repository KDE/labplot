#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBCERF libcerf QUIET)

find_library(LIBCERF_LIBRARIES
    NAMES cerf
    HINTS ${PC_LIBCERF_LIBRARY_DIRS}
)

find_path(LIBCERF_INCLUDE_DIR
    NAMES cerf.h
    HINTS ${PC_LIBCERF_INCLUDE_DIRS}
)

set(LIBCERF_VERSION ${PC_LIBCERF_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBCERF
    REQUIRED_VARS
        LIBCERF_LIBRARIES
        LIBCERF_INCLUDE_DIR
    VERSION_VAR
        LIBCERF_VERSION
)

if(LIBCERF_FOUND AND NOT TARGET libcerf::libcerf)
    add_library(libcerf::libcerf UNKNOWN IMPORTED)
    set_target_properties(libcerf::libcerf PROPERTIES
	 IMPORTED_LOCATION "${LIBCERF_LIBRARIES}"
         INTERFACE_COMPILE_OPTIONS "${PC_LIBCERF_CFLAGS}"
         INTERFACE_INCLUDE_DIRECTORIES "${LIBCERF_INCLUDE_DIR}"
    )
else()
	set(LIBCERF_LIBRARIES "")
endif()

mark_as_advanced(LIBCERF_LIBRARIES LIBCERF_INCLUDE_DIR LIBCERF_VERSION)

include(FeatureSummary)
set_package_properties(LIBCERF PROPERTIES
    DESCRIPTION "Efficient and accurate implementation of complex error functions, along with Dawson, Faddeeva, and Voigt functions"
    URL "https://jugit.fz-juelich.de/mlz/libcerf"
)
