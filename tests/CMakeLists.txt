set(SRC_DIR ${labplot2_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${SRC_DIR} ${GSL_INCLUDE_DIR})

# shared code
add_library(labplot2test STATIC CommonTest.cpp CommonMetaTest.cpp analysis/AnalysisTest.h nsl/NSLTest.h ../src/backend/lib/Debug.cpp)
target_link_libraries(labplot2test Qt${QT_MAJOR_VERSION}::Test KF${KF_MAJOR_VERSION}::ConfigCore Qt${QT_MAJOR_VERSION}::Widgets KF${KF_MAJOR_VERSION}::I18n)
if (KUserFeedback_FOUND AND QT_MAJOR_VERSION EQUAL 6)
	target_link_libraries( labplot2test KUserFeedbackCoreQt6 KUserFeedbackWidgetsQt6 )
endif ()
target_compile_definitions(labplot2test PUBLIC ${LABPLOT_COMPILE_DEFINITIONS})

add_subdirectory(analysis)
add_subdirectory(backend)
add_subdirectory(cartesianplot)
add_subdirectory(import_export)
add_subdirectory(nsl)
add_subdirectory(spreadsheet)
add_subdirectory(livedata)
add_subdirectory(matrix)
add_subdirectory(multirange)
add_subdirectory(commonfrontend)

IF (Cantor_FOUND)
	add_subdirectory(notebook)
ENDIF ()


option(ENABLE_TEST_DEPENDENCIES "Enable Dependencies Tests" ON)
if (ENABLE_TEST_DEPENDENCIES)
	add_executable (DependenciesTest DependenciesTest.cpp)
	target_link_libraries(DependenciesTest labplot2test labplot2lib)
	add_test(NAME DependenciesTest COMMAND DependenciesTest)
endif()


