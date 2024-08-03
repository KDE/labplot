add_executable(NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest labplottest ${FFTW3_LIBRARIES})

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
