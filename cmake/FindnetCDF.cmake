#=============================================================================
# Copyright (c) 2019 Harald Sitter <sitter@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# Try to find via config. If that isn't available fall back to manual lookup.
# Config is vastly preferrable because it will also make sure link dependencies
# are found and actually in the target link interface.
find_package(netCDF ${netCDF_FIND_VERSION} ${netCDF_FIND_REQUIRED} CONFIG QUIET)
if(netCDF_FOUND)
	MESSAGE (STATUS "Found netCDF: ${netCDF_INCLUDE_DIR}, ${netCDF_LIBRARIES} (found version \"${netCDF_VERSION}\")")
    return()
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
endif()

mark_as_advanced(netCDF_LIBRARIES netCDF_INCLUDE_DIR netCDF_VERSION)

include(FeatureSummary)

