option(ENABLE_TEST_BACKEND_RANGE "Enable Range Tests" ON)

if(ENABLE_TEST_BACKEND_RANGE)
    add_executable (RangeTest RangeTest.cpp)

    target_link_libraries(RangeTest labplotbackendlib labplotlib labplottest)

    add_test(NAME RangeTest COMMAND RangeTest)
endif()
