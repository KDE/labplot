INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (hilberttransformtest HilbertTransformTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(hilberttransformtest Qt5::Test)
target_link_libraries(hilberttransformtest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(hilberttransformtest KDMacTouchBar)
ENDIF ()

IF (FFTW3_FOUND)
	target_link_libraries(hilberttransformtest ${FFTW3_LIBRARIES})
ENDIF ()

target_link_libraries(hilberttransformtest labplot2lib)

add_test(NAME hilberttransformtest COMMAND hilberttransformtest)
