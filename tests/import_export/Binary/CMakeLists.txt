add_executable (BinaryFilterTest BinaryFilterTest.cpp)

target_link_libraries(BinaryFilterTest labplot2lib labplot2test)

add_test(NAME BinaryFilterTest COMMAND BinaryFilterTest)
