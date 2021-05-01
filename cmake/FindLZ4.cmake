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
