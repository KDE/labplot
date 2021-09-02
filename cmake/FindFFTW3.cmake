#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_FFTW3 fftw3 QUIET)

find_library(FFTW3_LIBRARIES
    NAMES fftw3
    HINTS ${PC_FFTW3_LIBRARY_DIRS}
)

find_path(FFTW3_INCLUDE_DIR
    NAMES fftw3.h
    HINTS ${PC_FFTW3_INCLUDE_DIRS}
    PATH_SUFFIXES fftw3
)

set(FFTW3_VERSION ${PC_FFTW3_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW3
    REQUIRED_VARS
        FFTW3_LIBRARIES
        FFTW3_INCLUDE_DIR
    VERSION_VAR
        FFTW3_VERSION
)

if(FFTW3_FOUND AND NOT TARGET FFTW3::FFTW3)
    add_library(FFTW3::FFTW3 UNKNOWN IMPORTED)
    set_target_properties(FFTW3::FFTW3 PROPERTIES
        IMPORTED_LOCATION "${FFTW3_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${PC_FFTW3_CFLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR}"
    )
else()
    set(FFTW3_LIBRARIES "")
endif()

mark_as_advanced(FFTW3_LIBRARIES FFTW3_INCLUDE_DIR FFTW3_VERSION)

include(FeatureSummary)
set_package_properties(FFTW3 PROPERTIES
    DESCRIPTION "Computing the Discrete Fourier Transform in one or more dimensions"
    URL "http://fftw.org/"
)

