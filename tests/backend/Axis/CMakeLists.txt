option(ENABLE_TEST_BACKEND_AXIS "Enable Axis Tests" ON)

if (ENABLE_TEST_BACKEND_AXIS)

    add_executable (AxisTest AxisTest.cpp)

    target_link_libraries(AxisTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME AxisTest COMMAND AxisTest)

endif()
