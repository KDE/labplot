INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (differentiationtest DifferentiationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(differentiationtest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(differentiationtest KDMacTouchBar)
ENDIF ()

target_link_libraries(differentiationtest labplot2lib)

add_test(NAME differentiationtest COMMAND differentiationtest)
