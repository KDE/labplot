add_executable (HistogramTest HistogramTest.cpp)

target_link_libraries(HistogramTest labplot2backendlib labplot2test labplot2lib labplot2nsllib)

add_test(NAME HistogramTest COMMAND HistogramTest)
