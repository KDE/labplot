add_executable(AsciiFilterTest AsciiFilterTest.cpp)

target_link_libraries(AsciiFilterTest labplotmetatest)

add_test(NAME AsciiFilterTest COMMAND AsciiFilterTest)
