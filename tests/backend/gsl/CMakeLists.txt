option(ENABLE_TEST_BACKEND_GSL "Enable GSL Tests" ON)

if(ENABLE_TEST_BACKEND_GSL)
    # Currently no test is implemented so disable it for now and keep it
    #add_executable(GSLTest GSLTest.cpp)

    #target_link_libraries(GSLTest labplotlib labplottest)

    #add_test(NAME GSLTest COMMAND GSLTest)

    add_executable(ExpressionParserTest ExpressionParserTest.cpp)

    target_link_libraries(ExpressionParserTest labplottest labplotbackendlib)

    add_test(NAME ExpressionParserTest COMMAND ExpressionParserTest)
endif()
