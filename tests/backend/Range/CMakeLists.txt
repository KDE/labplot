add_executable (RangeTest RangeTest.cpp)

target_link_libraries(RangeTest Qt5::Test labplot2lib labplot2test)

add_test(NAME RangeTest COMMAND RangeTest)
