add_executable (BarPlotTest BarPlotTest.cpp)

target_link_libraries(BarPlotTest Qt5::Test labplot2lib labplot2test)

add_test(NAME BarPlotTest COMMAND BarPlotTest)
