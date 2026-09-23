option(ENABLE_TEST_BACKEND_HEATMAP "Enable Heatmap Tests" ON)

if(ENABLE_TEST_BACKEND_HEATMAP)
    add_executable (HeatmapTest HeatmapTest.cpp)

    target_link_libraries(HeatmapTest labplotbackendlib labplotlib labplottest)

    add_test(NAME HeatmapTest COMMAND HeatmapTest)
endif()
