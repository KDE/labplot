# - Try to find CFITSIO
# Once done this will define
#
#  CFITSIO_FOUND - system has CFITSIO
#  CFITSIO_INCLUDE_DIR - the CFITSIO include directory
#  CFITSIO_LIBRARIES - Link these to use CFITSIO

# SPDX-FileCopyrightText: 2006 Jasem Mutlaq <mutlaqja@ikarustech.com>
# Based on FindLibfacile by Carsten Niehaus, <cniehaus@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

if (NOT WIN32)
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PC_CFITSIO cfitsio QUIET)
        set(CFITSIO_VERSION ${PC_CFITSIO_VERSION})
    endif ()
endif ()

if (CFITSIO_INCLUDE_DIR AND CFITSIO_LIBRARIES)
    # Already in cache
    set(CFITSIO_FOUND TRUE)
    message(STATUS "Found CFITSIO: ${CFITSIO_LIBRARIES}, ${CFITSIO_INCLUDE_DIR} (found version \"${CFITSIO_VERSION}\")")
else ()
    find_path(CFITSIO_INCLUDE_DIR fitsio.h
            ${PC_CFITSIO_INCLUDE_DIRS}
            ${_obIncDir}
            ${GNUWIN32_DIR}/include
    )

    find_library(CFITSIO_LIBRARIES NAMES cfitsio libcfitsio QUIET
            PATHS
            ${PC_CFITSIO_LIBRARY_DIRS}
            ${_obIncDir}
            ${GNUWIN32_DIR}/include
    )

    if(CFITSIO_INCLUDE_DIR AND CFITSIO_LIBRARIES)
        set(CFITSIO_FOUND TRUE)
    else ()
        set(CFITSIO_FOUND FALSE)
    endif()

    if (CFITSIO_FOUND)
        if (NOT CFitsio_FIND_QUIETLY)
		message(STATUS "Found CFITSIO: ${CFITSIO_LIBRARIES}, ${CFITSIO_INCLUDE_DIR} (found version \"${CFITSIO_VERSION}\")")
        endif (NOT CFitsio_FIND_QUIETLY)
    else ()
        if (CFitsio_FIND_REQUIRED)
            message(FATAL_ERROR "CFITSIO not found. Please install libcfitsio-dev or cfitsio-devel and try again.")
        endif ()
        set(CFITSIO_LIBRARIES "")
    endif ()


    mark_as_advanced(CFITSIO_INCLUDE_DIR CFITSIO_LIBRARIES CFITSIO_VERSION)

endif ()
