INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (smoothtest SmoothTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(smoothtest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(smoothtest KDMacTouchBar)
ENDIF ()

target_link_libraries(smoothtest labplot2lib)

add_test(NAME smoothtest COMMAND smoothtest)
