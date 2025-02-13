set(SRC_DIR ${labplot_SOURCE_DIR}/src)
include_directories(${SRC_DIR} ${GSL_INCLUDE_DIR})

# shared code
add_library(labplottest STATIC CommonTest.cpp analysis/AnalysisTest.cpp nsl/NSLTest.h ../src/backend/lib/Debug.cpp)
target_link_libraries(labplottest labplotnsllib Qt${QT_MAJOR_VERSION}::Test KF${KF_MAJOR_VERSION}::ConfigCore KF${KF_MAJOR_VERSION}::I18n)
add_library(labplotmetatest STATIC CommonMetaTest.cpp)
target_link_libraries(labplotmetatest labplotbackendlib labplotlib labplottest)

if(KUserFeedback_FOUND AND QT_MAJOR_VERSION EQUAL 6)
    target_link_libraries(labplottest KUserFeedbackCoreQt6 KUserFeedbackWidgetsQt6)
endif()
target_compile_definitions(labplottest PUBLIC ${LABPLOT_COMPILE_DEFINITIONS})

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

if(Cantor_FOUND)
    add_subdirectory(notebook)
endif()

option(ENABLE_TEST_DEPENDENCIES "Enable Dependencies Tests" ON)
if(ENABLE_TEST_DEPENDENCIES)
    add_executable (DependenciesTest DependenciesTest.cpp)
    target_link_libraries(DependenciesTest labplotbackendlib labplotlib labplottest)
    add_test(NAME DependenciesTest COMMAND DependenciesTest)
endif()
