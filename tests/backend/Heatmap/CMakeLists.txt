add_executable (HeatmapTest HeatmapTest.cpp)

target_link_libraries(HeatmapTest labplot2backendlib labplot2lib labplot2test)

add_test(NAME HeatmapTest COMMAND HeatmapTest)
