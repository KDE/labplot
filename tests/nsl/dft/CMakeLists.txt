add_executable(NSLDFTTest NSLDFTTest.cpp)

target_link_libraries(NSLDFTTest labplottest)

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
