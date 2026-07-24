option(ENABLE_TEST_MULTIRANGE "Enable Multirange Tests" ON)

if(ENABLE_TEST_MULTIRANGE)
    add_executable (MultiRangeTest MultiRangeTest.cpp)
    target_link_libraries(MultiRangeTest labplotbackendlib labplotlib labplottest)
    add_test(NAME MultiRangeTest COMMAND MultiRangeTest)
endif()
