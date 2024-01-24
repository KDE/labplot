option(ENABLE_TEST_MULTIRANGE "Enable Multirange Tests" ON)

if (ENABLE_TEST_MULTIRANGE)
    add_executable (MultiRangeTest MultiRangeTest.cpp)

    target_link_libraries(MultiRangeTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME MultiRangeTest COMMAND MultiRangeTest)

    add_executable (MultiRangeTest2 MultiRangeTest2.cpp)

    target_link_libraries(MultiRangeTest2 labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME MultiRangeTest2 COMMAND MultiRangeTest2)
endif()
