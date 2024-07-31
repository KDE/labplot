option(ENABLE_TEST_BACKEND_PARSER "Enable Parser Tests" ON)

if(ENABLE_TEST_BACKEND_PARSER)
    add_executable (ParserTest ParserTest.cpp)

    target_link_libraries(ParserTest labplot2test labplot2backendlib)

    add_test(NAME ParserTest COMMAND ParserTest)
endif()
