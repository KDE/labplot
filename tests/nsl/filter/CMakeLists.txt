add_executable (NSLFilterTest NSLFilterTest.cpp)

target_link_libraries(NSLFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
