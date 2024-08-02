add_executable(NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest labplottest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES})

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
