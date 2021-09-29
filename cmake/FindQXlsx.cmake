#=============================================================================
# SPDX-FileCopyrightText: 2021 Fabian Kristof <f-kristof@hotmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_library(QXLSX_LIBRARIES NAMES QXlsx)

find_path(QXLSX_INCLUDE_DIR xlsxdocument.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QXlsx
    FOUND_VAR
    	QXLSX_FOUND
    REQUIRED_VARS
        QXLSX_LIBRARIES
	QXLSX_INCLUDE_DIR
)

if(QXLSX_FOUND)
    add_library(QXlsx UNKNOWN IMPORTED)
    set_target_properties(QXlsx PROPERTIES
        IMPORTED_LOCATION "${QXLSX_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${QXLSX_INCLUDE_DIR}"
    )
else()
	set(QXLSX_LIBRARIES "")
endif()

mark_as_advanced(QXLSX_LIBRARIES QXLSX_INCLUDE_DIR)

include(FeatureSummary)
set_package_properties(QXlsx PROPERTIES
    DESCRIPTION "QXlsx is excel file(*.xlsx) reader/writer library."
    URL "https://github.com/QtExcel/QXlsx"
)
