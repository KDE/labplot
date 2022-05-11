add_executable (NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
