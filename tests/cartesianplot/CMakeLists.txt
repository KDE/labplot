add_executable (CartesianPlotTest CartesianPlotTest.cpp)

target_link_libraries(CartesianPlotTest labplot2lib Qt5::Test)

add_test(NAME CartesianPlotTest COMMAND CartesianPlotTest)
