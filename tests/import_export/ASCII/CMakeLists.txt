add_executable(AsciiFilterTest AsciiFilterTest.cpp)

target_link_libraries(AsciiFilterTest labplot2metatest)

add_test(NAME AsciiFilterTest COMMAND AsciiFilterTest)
