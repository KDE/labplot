option(ENABLE_TEST_CARTESIANPLOT "Enable CartesianPlot Tests" ON)

if(ENABLE_TEST_CARTESIANPLOT)
    add_executable (CartesianPlotTest CartesianPlotTest.cpp)
    target_link_libraries(CartesianPlotTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
    add_test(NAME CartesianPlotTest COMMAND CartesianPlotTest)

    add_subdirectory(plots)
endif()
