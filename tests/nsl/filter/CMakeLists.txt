add_executable(NSLFilterTest NSLFilterTest.cpp)

target_link_libraries(NSLFilterTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES} labplot2test)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
