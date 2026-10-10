option(ENABLE_TEST_BACKEND_INFOELEMENT "Enable InfoElement Tests" ON)

if(ENABLE_TEST_BACKEND_INFOELEMENT)
    add_executable (InfoElementTest InfoElementTest.cpp)

    target_link_libraries(InfoElementTest labplotbackendlib labplotlib labplotmetatest)

    add_test(NAME InfoElementTest COMMAND InfoElementTest)
endif()
