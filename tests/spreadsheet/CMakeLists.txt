INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

add_executable (spreadsheettest SpreadsheetTest.cpp)

target_link_libraries(spreadsheettest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(spreadsheettest KDMacTouchBar)
ENDIF ()
target_link_libraries(spreadsheettest labplot2lib)

add_test(NAME spreadsheettest COMMAND spreadsheettest)
