INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (parsertest ParserTest.cpp ../../CommonTest.cpp)

target_link_libraries(parsertest Qt5::Test)

target_link_libraries(parsertest labplot2lib)

add_test(NAME parsertest COMMAND parsertest)
