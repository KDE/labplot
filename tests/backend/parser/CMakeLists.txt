INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (parsertest ParserTest.cpp ../../CommonTest.cpp)

target_link_libraries(parsertest Qt5::Test labplot2lib)

add_test(NAME parsertest COMMAND parsertest)
