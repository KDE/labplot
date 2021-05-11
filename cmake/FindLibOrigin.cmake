#=============================================================================
# Copyright (c) 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
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

