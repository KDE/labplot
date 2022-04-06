add_executable (AsciiFilterTest AsciiFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(AsciiFilterTest labplot2lib Qt5::Test)

add_test(NAME AsciiFilterTest COMMAND AsciiFilterTest)
