add_executable (ConvolutionTest ConvolutionTest.cpp)

target_link_libraries(ConvolutionTest labplot2lib Qt5::Test labplot2test)

add_test(NAME ConvolutionTest COMMAND ConvolutionTest)
