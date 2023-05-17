add_executable (NSLBaselineTest NSLBaselineTest.cpp)

target_link_libraries(NSLBaselineTest labplot2lib labplot2test)

add_test(NAME NSLBaselineTest COMMAND NSLBaselineTest)
