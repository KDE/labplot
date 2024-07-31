option(ENABLE_TEST_BACKEND_GSL "Enable GSL Tests" ON)

if(ENABLE_TEST_BACKEND_GSL)
    # Currently no test is implemented so disable it for now and keep it
    #add_executable(GSLTest GSLTest.cpp)

    #target_link_libraries(GSLTest labplot2lib labplot2test)

    #add_test(NAME GSLTest COMMAND GSLTest)

    add_executable(ExpressionParserTest ExpressionParserTest.cpp)

    target_link_libraries(ExpressionParserTest labplot2test labplot2backendlib labplot2nsllib)

    add_test(NAME ExpressionParserTest COMMAND ExpressionParserTest)
endif()
