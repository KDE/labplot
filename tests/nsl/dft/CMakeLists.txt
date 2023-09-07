add_executable (NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES} labplot2test)

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
