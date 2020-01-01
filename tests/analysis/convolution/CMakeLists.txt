INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (convolutiontest ConvolutionTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(convolutiontest Qt5::Test)
target_link_libraries(convolutiontest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(convolutiontest KDMacTouchBar)
ENDIF ()

IF (FFTW3_FOUND)
	target_link_libraries(convolutiontest ${FFTW3_LIBRARIES})
ENDIF ()

target_link_libraries(convolutiontest labplot2lib)

add_test(NAME convolutiontest COMMAND convolutiontest)
