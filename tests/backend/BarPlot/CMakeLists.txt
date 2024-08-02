option(ENABLE_TEST_BACKEND_BARPLOT "Enable Barplot Tests" ON)

if(ENABLE_TEST_BACKEND_BARPLOT)
    add_executable (BarPlotTest BarPlotTest.cpp)

    target_link_libraries(BarPlotTest labplotbackendlib labplottest labplotlib)

    add_test(NAME BarPlotTest COMMAND BarPlotTest)
endif()
