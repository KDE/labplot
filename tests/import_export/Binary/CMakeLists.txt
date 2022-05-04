add_executable (BinaryFilterTest BinaryFilterTest.cpp)

target_link_libraries(BinaryFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME BinaryFilterTest COMMAND BinaryFilterTest)
