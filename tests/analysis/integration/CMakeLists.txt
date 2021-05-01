INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (integrationtest IntegrationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(integrationtest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(integrationtest KDMacTouchBar)
ENDIF ()

target_link_libraries(integrationtest labplot2lib)

add_test(NAME integrationtest COMMAND integrationtest)
