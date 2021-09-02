#=============================================================================
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

# Try to find via config. If that isn't available fall back to manual lookup.
# Config is vastly preferable because it will also make sure link dependencies
# are found and actually in the target link interface.
find_package(Cantor ${Cantor_FIND_VERSION} ${Cantor_FIND_REQUIRED} CONFIG QUIET)
if(Cantor_FOUND)
    return()
endif()

find_library(Cantor_LIBRARIES cantorlibs)
find_path(Cantor_INCLUDE_DIR cantor/worksheetaccess.h)

if(EXISTS "${Cantor_INCLUDE_DIR}/cantor/cantorlibs_version.h")
    file(READ "${Cantor_INCLUDE_DIR}/cantor/cantorlibs_version.h" Cantorlibs_version_H_CONTENT)

    string(REGEX MATCH "#define CANTOR_VERSION_MAJOR[ ]+[0-9]+" Cantor_VERSION_MAJOR_MATCH ${Cantorlibs_version_H_CONTENT})
    string(REGEX MATCH "#define CANTOR_VERSION_MINOR[ ]+[0-9]+" Cantor_VERSION_MINOR_MATCH ${Cantorlibs_version_H_CONTENT})
    string(REGEX MATCH "#define CANTOR_VERSION_PATCH[ ]+[0-9]+" Cantor_VERSION_PATCH_MATCH ${Cantorlibs_version_H_CONTENT})

    string(REGEX REPLACE ".*_MAJOR[ ]+(.*)" "\\1" Cantor_VERSION_MAJOR ${Cantor_VERSION_MAJOR_MATCH})
    string(REGEX REPLACE ".*_MINOR[ ]+(.*)" "\\1" Cantor_VERSION_MINOR ${Cantor_VERSION_MINOR_MATCH})
    string(REGEX REPLACE ".*_PATCH[ ]+(.*)" "\\1"  Cantor_VERSION_PATCH  ${Cantor_VERSION_PATCH_MATCH})

    set(Cantor_VERSION "${Cantor_VERSION_MAJOR}.${Cantor_VERSION_MINOR}.${Cantor_VERSION_PATCH}")
else()
    set(Cantor_VERSION "0.0.0")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cantor
    FOUND_VAR
        Cantor_FOUND
    REQUIRED_VARS
        Cantor_LIBRARIES
        Cantor_INCLUDE_DIR
    VERSION_VAR
        Cantor_VERSION
)

if(Cantor_FOUND AND NOT TARGET Cantor::cantorlibs)
    add_library(Cantor::cantorlibs UNKNOWN IMPORTED)
    set_target_properties(Cantor::cantorlibs PROPERTIES
        IMPORTED_LOCATION "${Cantor_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Cantor_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Cantor_LIBRARIES Cantor_INCLUDE_DIR Cantor_VERSION)

include(FeatureSummary)

