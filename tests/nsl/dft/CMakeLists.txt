add_executable(NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest ${FFTW3_LIBRARIES} labplottest)

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
