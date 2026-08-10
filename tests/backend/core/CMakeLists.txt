option(ENABLE_TEST_BACKEND_CORE "Enable Core Tests" ON)

if(ENABLE_TEST_BACKEND_CORE)
    add_executable (AbstractAspectTest AbstractAspectTest.cpp)

    target_link_libraries(AbstractAspectTest labplotbackendlib labplotlib labplottest)
    target_compile_definitions(AbstractAspectTest PRIVATE LABPLOT_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

    add_test(NAME AbstractAspectTest COMMAND AbstractAspectTest)
endif()
