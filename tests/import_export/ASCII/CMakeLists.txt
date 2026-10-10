add_executable(AsciiFilterTest AsciiFilterTest.cpp)

target_link_libraries(AsciiFilterTest labplottest)

add_test(NAME AsciiFilterTest COMMAND AsciiFilterTest)
