add_executable (AsciiFilterTest AsciiFilterTest.cpp)

target_link_libraries(AsciiFilterTest labplot2lib labplot2test)

add_test(NAME AsciiFilterTest COMMAND AsciiFilterTest)
