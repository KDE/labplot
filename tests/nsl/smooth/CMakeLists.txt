add_executable (NSLSmoothTest NSLSmoothTest.cpp)

target_link_libraries(NSLSmoothTest labplot2lib labplot2test)

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
