# - Find Orcus
# Find the Orcus filter library.
#
# This module defines
#  Orcus_FOUND - whether the Orcus library was found
#  Orcus_LIBRARIES - the Orcus library
#  Orcus_INCLUDE_DIR - the include path of the Orcus library

# SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
# SPDX-License-Identifier: BSD-3-Clause

if (Orcus_INCLUDE_DIR AND Orcus_LIBRARIES)
    # Already in cache
    set (Orcus_FOUND TRUE)
else ()
    find_package(PkgConfig QUIET)
    pkg_search_module(PC_ORCUS liborcus liborcus-0.21 liborcus-0.20 liborcus-0.19 liborcus-0.18 liborcus-0.17 liborcus-0.16 QUIET)
    pkg_search_module(PC_IXION libixion libixion-0.20 libixion-0.19 libixion-0.18 libixion-0.17 libixion-0.16 QUIET)

    find_library (Orcus_LIBRARY
        NAMES orcus orcus-0.21 orcus-0.20 orcus-0.19 orcus-0.18 orcus-0.17 orcus-0.16
        HINTS ${PC_ORCUS_LIBRARY_DIRS}
        PATH_SUFFIXES orcus
    )
    find_library (Orcus_parser_LIBRARY
        NAMES orcus-parser orcus-parser-0.21 orcus-parser-0.20 orcus-parser-0.19 orcus-parser-0.18 orcus-parser-0.17 orcus-parser-0.16
        HINTS ${PC_ORCUS_LIBRARY_DIRS}
        PATH_SUFFIXES orcus
    )
    find_library (Orcus_spreadsheet_LIBRARY
        NAMES orcus-spreadsheet-model orcus-spreadsheet-model-0.21 orcus-spreadsheet-model-0.20 orcus-spreadsheet-model-0.19 orcus-spreadsheet-model-0.18 orcus-spreadsheet-model-0.17 orcus-spreadsheet-model-0.16
        HINTS ${PC_ORCUS_LIBRARY_DIRS}
        PATH_SUFFIXES orcus
    )
    set(Orcus_LIBRARIES ${Orcus_LIBRARY} ${Orcus_parser_LIBRARY} ${Orcus_spreadsheet_LIBRARY})
    if (NOT DEFINED Orcus_LIBRARY)
	message(STATUS "Orcus library not found")
    endif ()
    if (NOT DEFINED Orcus_parser_LIBRARY)
	message(STATUS "Orcus parser library not found")
    endif ()
    if (NOT DEFINED Orcus_spreadsheet_LIBRARY)
	message(STATUS "Orcus spreadsheet library not found")
    endif ()

    find_library (Ixion_LIBRARY
        NAMES ixion ixion-0.20 ixion-0.19 ixion-0.18 ixion-0.17 ixion-0.16
	HINTS ${PC_IXION_LIBRARY_DIRS}
        PATH_SUFFIXES orcus
    )
    find_path (Orcus_INCLUDE_DIR
        NAMES orcus/orcus_ods.hpp
        HINTS ${PC_ORCUS_INCLUDE_DIRS}
        PATH_SUFFIXES orcus
    )
    find_path (Ixion_INCLUDE_DIR
        NAMES ixion/info.hpp
	HINTS ${PC_IXION_INCLUDE_DIRS}
        PATH_SUFFIXES ixion
    )
    if (NOT DEFINED Ixion_INCLUDE_DIR)
	    message(STATUS "Ixion includes not found")
    endif ()
    if (NOT DEFINED Orcus_INCLUDE_DIR)
	message(STATUS "Orcus includes not found")
    endif ()

    set(LIBORCUS_VERSION ${PC_ORCUS_VERSION})
    set(LIBIXION_VERSION ${PC_IXION_VERSION})
    # latest version
    if (NOT DEFINED LIBORCUS_VERSION)
        set(LIBORCUS_VERSION "0.21.0")
    endif()
    if (NOT DEFINED LIBIXION_VERSION)
            set(LIBIXION_VERSION "0.20.0")
    endif()

    include (FindPackageHandleStandardArgs)
    find_package_handle_standard_args (Orcus DEFAULT_MSG Orcus_LIBRARIES Orcus_INCLUDE_DIR Ixion_INCLUDE_DIR LIBORCUS_VERSION)
endif ()

mark_as_advanced(Orcus_INCLUDE_DIR Ixion_INCLUDE_DIR Orcus_LIBRARY Orcus_parser_LIBRARY Orcus_spreadsheet_LIBRARY Orcus_LIBRARIES Ixion_LIBRARY LIBORCUS_VERSION LIBIXION_VERSION)

if (Orcus_FOUND)
   add_library(Orcus UNKNOWN IMPORTED)
   set_target_properties(Orcus PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${Orcus_INCLUDE_DIR} IMPORTED_LOCATION "${Orcus_LIBRARIES} ${Ixion_LIBRARY}")
endif()
