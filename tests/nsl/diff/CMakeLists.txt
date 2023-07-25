add_executable (NSLDiffTest NSLDiffTest.cpp)

target_link_libraries(NSLDiffTest labplot2lib labplot2test)

add_test(NAME NSLDiffTest COMMAND NSLDiffTest)
