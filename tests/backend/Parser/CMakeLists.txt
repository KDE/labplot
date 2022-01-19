INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (ParserTest ParserTest.cpp ../../CommonTest.cpp)

target_link_libraries(ParserTest Qt5::Test labplot2lib)

add_test(NAME ParserTest COMMAND ParserTest)
