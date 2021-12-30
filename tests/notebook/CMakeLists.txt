INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

add_executable (notebooktest NotebookTest.cpp)

target_link_libraries(notebooktest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(notebooktest KDMacTouchBar)
ENDIF ()
target_link_libraries(notebooktest labplot2lib)

add_test(NAME notebooktest COMMAND notebooktest)
