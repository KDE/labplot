add_executable (correlationcoefficienttest CorrelationCoefficientTest.cpp ../../CommonTest.cpp)

target_link_libraries(correlationcoefficienttest Qt5::Test)
target_link_libraries(correlationcoefficienttest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

IF (FFTW_FOUND)
	target_link_libraries(correlationcoefficienttest ${FFTW_LIBRARIES})
ENDIF ()

target_link_libraries(correlationcoefficienttest labplot2lib)

add_test(NAME correlationcoefficienttest COMMAND correlationcoefficienttest)
