#=============================================================================
# SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
#
# SPDX-License-Identifier: BSD-3-Clause
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
