add_executable(NSLFilterTest NSLFilterTest.cpp)

target_link_libraries(NSLFilterTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES} labplot2test)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
