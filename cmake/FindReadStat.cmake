#=============================================================================
# SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
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

# readstat build on Windows creates readstat.dll.lib when shared libs are enabled
find_library(READSTAT_LIBRARIES NAMES readstat readstat.dll)

find_path(READSTAT_INCLUDE_DIR readstat.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ReadStat
    FOUND_VAR
        READSTAT_FOUND
    REQUIRED_VARS
    READSTAT_LIBRARIES
        READSTAT_INCLUDE_DIR
)

if(READSTAT_FOUND AND NOT TARGET readstat)
    add_library(readstat UNKNOWN IMPORTED)
    set_target_properties(readstat PROPERTIES
        IMPORTED_LOCATION "${READSTAT_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${READSTAT_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(READSTAT_LIBRARIES READSTAT_INCLUDE_DIR)

include(FeatureSummary)
set_package_properties(ReadStat PROPERTIES
    DESCRIPTION "A command-line tool and library for reading files from popular stats packages"
    URL "https://github.com/WizardMac/ReadStat"
)
