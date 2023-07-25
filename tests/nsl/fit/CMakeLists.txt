add_executable (NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest labplot2lib labplot2test)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
