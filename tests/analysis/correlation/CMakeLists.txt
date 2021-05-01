INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (correlationtest CorrelationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(correlationtest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES})
IF (APPLE)
	target_link_libraries(correlationtest KDMacTouchBar)
ENDIF ()

target_link_libraries(correlationtest labplot2lib)

add_test(NAME correlationtest COMMAND correlationtest)
