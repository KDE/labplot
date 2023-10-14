add_executable (BarPlotTest BarPlotTest.cpp)

target_link_libraries(BarPlotTest labplot2backendlib labplot2test labplot2lib labplot2nsllib)

add_test(NAME BarPlotTest COMMAND BarPlotTest)
