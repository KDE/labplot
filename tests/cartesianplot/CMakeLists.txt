option(ENABLE_TEST_CARTESIANPLOT "Enable CartesianPlot Tests" ON)
if(ENABLE_TEST_CARTESIANPLOT)
    add_executable (CartesianPlotTest CartesianPlotTest.cpp)
    target_link_libraries(CartesianPlotTest labplotlib labplotbackendlib labplottest)
    add_test(NAME CartesianPlotTest COMMAND CartesianPlotTest)

    add_subdirectory(plots)

    add_executable (CartesianCoordinateSystemTest CartesianCoordinateSystemTest.cpp)
    target_link_libraries(CartesianCoordinateSystemTest labplotlib labplotbackendlib labplottest)
    add_test(NAME CartesianCoordinateSystemTest COMMAND CartesianCoordinateSystemTest)
endif()
