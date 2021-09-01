#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
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
