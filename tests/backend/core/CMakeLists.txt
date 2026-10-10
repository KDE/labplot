option(ENABLE_TEST_BACKEND_CORE "Enable Core Tests" ON)

if(ENABLE_TEST_BACKEND_CORE)
    add_executable (AbstractAspectTest AbstractAspectTest.cpp)

    target_link_libraries(AbstractAspectTest labplotbackendlib labplotlib labplotmetatest)
    target_compile_definitions(AbstractAspectTest PRIVATE LABPLOT_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

    add_test(NAME AbstractAspectTest COMMAND AbstractAspectTest)

    add_executable (IntervalTest IntervalTest.cpp)
    target_link_libraries(IntervalTest labplotbackendlib labplotlib labplotmetatest)
    add_test(NAME IntervalTest COMMAND IntervalTest)

    add_executable (AbstractFilterTest AbstractFilterTest.cpp)
    target_link_libraries(AbstractFilterTest labplotbackendlib labplotlib labplotmetatest)
    add_test(NAME AbstractFilterTest COMMAND AbstractFilterTest)
endif()
