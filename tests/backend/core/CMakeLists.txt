option(ENABLE_TEST_BACKEND_CORE "Enable Core Tests" ON)

if(ENABLE_TEST_BACKEND_CORE)
    add_executable (AbstractAspectTest AbstractAspectTest.cpp)

    target_link_libraries(AbstractAspectTest labplot2backendlib labplot2lib labplot2test)

    add_test(NAME AbstractAspectTest COMMAND AbstractAspectTest)
endif()
