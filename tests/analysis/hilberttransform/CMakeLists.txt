INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (hilberttransformtest HilbertTransformTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(hilberttransformtest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES})
IF (APPLE)
	target_link_libraries(hilberttransformtest KDMacTouchBar)
ENDIF ()

target_link_libraries(hilberttransformtest labplot2lib)

add_test(NAME hilberttransformtest COMMAND hilberttransformtest)
