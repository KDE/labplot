option(ENABLE_TEST_BACKEND_AXIS "Enable Axis Tests" ON)

if(ENABLE_TEST_BACKEND_AXIS)
    add_executable(AxisTest AxisTest.cpp)
    target_link_libraries(AxisTest labplotbackendlib labplotlib labplottest)
    add_test(NAME AxisTest COMMAND AxisTest)
endif()
