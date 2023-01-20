add_executable (NSLBaselineTest NSLBaselineTest.cpp)

target_link_libraries(NSLBaselineTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLBaselineTest COMMAND NSLBaselineTest)
