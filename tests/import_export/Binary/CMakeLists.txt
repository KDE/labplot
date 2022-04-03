add_executable (BinaryFilterTest BinaryFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(BinaryFilterTest labplot2lib Qt5::Test)

add_test(NAME BinaryFilterTest COMMAND BinaryFilterTest)
