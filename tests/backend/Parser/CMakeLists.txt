add_executable (ParserTest ParserTest.cpp)

target_link_libraries(ParserTest labplot2backendlib labplot2nsllib labplot2test)

add_test(NAME ParserTest COMMAND ParserTest)
