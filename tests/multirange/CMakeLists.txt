option(ENABLE_TEST_MULTIRANGE "Enable Multirange Tests" ON)

if(ENABLE_TEST_MULTIRANGE)
    add_executable (MultiRangeTest MultiRangeTest.cpp)
    target_link_libraries(MultiRangeTest labplotbackendlib labplotlib labplottest)
    add_test(NAME MultiRangeTest COMMAND MultiRangeTest)

    add_executable (MultiRangeTest2 MultiRangeTest2.cpp)
    target_link_libraries(MultiRangeTest2 labplotbackendlib labplotlib labplottest)
    add_test(NAME MultiRangeTest2 COMMAND MultiRangeTest2)

    add_executable (MultiRangeTest3 MultiRangeTest3.cpp)
    target_link_libraries(MultiRangeTest3 labplotbackendlib labplotlib labplottest)
    add_test(NAME MultiRangeTest3 COMMAND MultiRangeTest3)
endif()
