INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

add_executable (CartesianPlotTest CartesianPlotTest.cpp)

target_link_libraries(CartesianPlotTest Qt5::Test KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(CartesianPlotTest KDMacTouchBar)
ENDIF ()
target_link_libraries(CartesianPlotTest labplot2lib)

add_test(NAME CartesianPlotTest COMMAND CartesianPlotTest)
