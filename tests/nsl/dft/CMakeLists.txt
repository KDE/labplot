add_executable (NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest labplot2lib labplot2test)

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
