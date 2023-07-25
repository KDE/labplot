add_executable (ParserTest ParserTest.cpp)

target_link_libraries(ParserTest labplot2lib labplot2test)

add_test(NAME ParserTest COMMAND ParserTest)
