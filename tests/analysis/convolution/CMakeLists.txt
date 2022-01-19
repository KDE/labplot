INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (ConvolutionTest ConvolutionTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(ConvolutionTest labplot2lib Qt5::Test)

add_test(NAME ConvolutionTest COMMAND ConvolutionTest)
