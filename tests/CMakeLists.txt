set(SRC_DIR ${labplot2_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${SRC_DIR} ${GSL_INCLUDE_DIR})

# shared code
add_library(labplot2test STATIC CommonTest.cpp analysis/AnalysisTest.cpp nsl/NSLTest.cpp)
target_link_libraries(labplot2test Qt5::Test KF5::ConfigCore Qt5::Widgets KF5::I18n)

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
