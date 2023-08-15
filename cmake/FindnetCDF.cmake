#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

# Try to find via config. If that isn't available fall back to manual lookup.
# Config is vastly preferable because it will also make sure link dependencies
# are found and actually in the target link interface.
if(NOT MSVC_FOUND AND NOT APPLE)
    find_package(netCDF ${netCDF_FIND_VERSION} ${netCDF_FIND_REQUIRED} CONFIG QUIET)
    if(netCDF_FOUND)
        MESSAGE (STATUS "Found netCDF: ${netCDF_INCLUDE_DIR}, ${netCDF_LIBRARIES} (found version \"${netCDF_VERSION}\")")
        return()
    endif()
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_netCDF netcdf QUIET)

find_library(netCDF_LIBRARIES
    NAMES netcdf
    HINTS ${PC_netCDF_LIBRARY_DIRS}
)

find_path(netCDF_INCLUDE_DIR
    NAMES netcdf.h
    HINTS ${PC_netCDF_INCLUDE_DIRS}
)

set(netCDF_VERSION ${PC_netCDF_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(netCDF
    FOUND_VAR
        netCDF_FOUND
    REQUIRED_VARS
        netCDF_LIBRARIES
        netCDF_INCLUDE_DIR
    VERSION_VAR
        netCDF_VERSION
)

if(netCDF_FOUND AND NOT TARGET netcdf)
    add_library(netcdf UNKNOWN IMPORTED)
    set_target_properties(netcdf PROPERTIES
        IMPORTED_LOCATION "${netCDF_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${netCDF_INCLUDE_DIR}"
    )
else()
    set(netCDF_LIBRARIES "")
endif()

mark_as_advanced(netCDF_LIBRARIES netCDF_INCLUDE_DIR netCDF_VERSION)

include(FeatureSummary)

