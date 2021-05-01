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

find_library(MATIO_LIBRARIES matio)

find_path(MATIO_INCLUDE_DIR matio.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Matio
    FOUND_VAR
    	MATIO_FOUND
    REQUIRED_VARS
        MATIO_LIBRARIES
	MATIO_INCLUDE_DIR
)

if(MATIO_FOUND)
    add_library(matio UNKNOWN IMPORTED)
    set_target_properties(matio PROPERTIES
        IMPORTED_LOCATION "${MATIO_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${MATIO_INCLUDE_DIR}"
    )
else()
	set(MATIO_LIBRARIES "")
endif()

mark_as_advanced(MATIO_LIBRARIES MATIO_INCLUDE_DIR)

include(FeatureSummary)
set_package_properties(Matio PROPERTIES
    DESCRIPTION "Reading and writing binary MATLAB MAT files"
    URL "https://github.com/tbeu/matio"
)
