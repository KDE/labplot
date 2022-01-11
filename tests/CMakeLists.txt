set(SRC_DIR ${labplot2_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${SRC_DIR})

add_subdirectory(analysis)
add_subdirectory(backend)
add_subdirectory(cartesianplot)
add_subdirectory(import_export)
add_subdirectory(nsl)
add_subdirectory(spreadsheet)
add_subdirectory(multirange)

IF (Cantor_FOUND)
	add_subdirectory(notebook)
ENDIF ()
