add_executable (ParserTest ParserTest.cpp)

target_link_libraries(ParserTest labplot2test labplot2backendlib labplot2nsllib)

add_test(NAME ParserTest COMMAND ParserTest)
