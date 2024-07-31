option(ENABLE_TEST_BACKEND_BARPLOT "Enable Barplot Tests" ON)

if(ENABLE_TEST_BACKEND_BARPLOT)
    add_executable (BarPlotTest BarPlotTest.cpp)

    target_link_libraries(BarPlotTest labplot2backendlib labplot2test labplot2lib)

    add_test(NAME BarPlotTest COMMAND BarPlotTest)
endif()
