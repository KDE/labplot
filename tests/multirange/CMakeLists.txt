INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

add_executable (MultiRangeTest MultiRangeTest.cpp)

target_link_libraries(MultiRangeTest Qt5::Test KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(MultiRangeTest KDMacTouchBar)
ENDIF ()
target_link_libraries(MultiRangeTest labplot2lib)

add_test(NAME MultiRangeTest COMMAND MultiRangeTest)
