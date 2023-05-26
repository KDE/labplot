set(SRC_DIR ${labplot2_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${SRC_DIR} ${GSL_INCLUDE_DIR})

# shared code
add_library(labplot2test STATIC CommonTest.cpp analysis/AnalysisTest.cpp nsl/NSLTest.cpp)
target_link_libraries(labplot2test Qt${QT_MAJOR_VERSION}::Test KF${KF_MAJOR_VERSION}::ConfigCore Qt${QT_MAJOR_VERSION}::Widgets KF${KF_MAJOR_VERSION}::I18n)
target_compile_definitions(labplot2test PUBLIC ${LABPLOT_COMPILE_DEFINITIONS})

add_subdirectory(analysis)
add_subdirectory(backend)
add_subdirectory(cartesianplot)
add_subdirectory(import_export)
add_subdirectory(nsl)
add_subdirectory(spreadsheet)
add_subdirectory(matrix)
add_subdirectory(multirange)
add_subdirectory(commonfrontend)

IF (Cantor_FOUND)
	add_subdirectory(notebook)
ENDIF ()
