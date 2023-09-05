add_executable (BarPlotTest BarPlotTest.cpp)

target_link_libraries(BarPlotTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME BarPlotTest COMMAND BarPlotTest)
