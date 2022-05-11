add_executable (ParserTest ParserTest.cpp)

target_link_libraries(ParserTest Qt5::Test labplot2lib labplot2test)

add_test(NAME ParserTest COMMAND ParserTest)
