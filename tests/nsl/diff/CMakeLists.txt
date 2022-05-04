add_executable (NSLDiffTest NSLDiffTest.cpp)

target_link_libraries(NSLDiffTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLDiffTest COMMAND NSLDiffTest)
