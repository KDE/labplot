option(ENABLE_TEST_TIME_SERIES_ANALYSIS "Enable Time Series Analysis Tests" ON)

if(ENABLE_TEST_TIME_SERIES_ANALYSIS)
    add_subdirectory(decomposition)
endif()
