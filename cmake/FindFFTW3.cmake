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
endif()

mark_as_advanced(FFTW3_LIBRARIES FFTW3_INCLUDE_DIR FFTW3_VERSUON)

include(FeatureSummary)
set_package_properties(FFTW3 PROPERTIES
    DESCRIPTION "Computing the Discrete Fourier Transform in one or more dimensions"
    URL "http://fftw.org/"
)
