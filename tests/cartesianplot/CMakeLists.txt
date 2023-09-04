add_executable (CartesianPlotTest CartesianPlotTest.cpp)

target_link_libraries(CartesianPlotTest labplot2lib labplot2test)

add_test(NAME CartesianPlotTest COMMAND CartesianPlotTest)

add_subdirectory(plots)
