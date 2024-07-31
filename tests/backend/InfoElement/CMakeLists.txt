option(ENABLE_TEST_BACKEND_INFOELEMENT "Enable InfoElement Tests" ON)

if(ENABLE_TEST_BACKEND_INFOELEMENT)
    add_executable (InfoElementTest InfoElementTest.cpp)

    target_link_libraries(InfoElementTest labplot2backendlib labplot2lib labplot2test)

    add_test(NAME InfoElementTest COMMAND InfoElementTest)
endif()
