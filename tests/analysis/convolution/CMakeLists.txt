add_executable (ConvolutionTest ConvolutionTest.cpp)

target_link_libraries(ConvolutionTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ConvolutionTest COMMAND ConvolutionTest)
