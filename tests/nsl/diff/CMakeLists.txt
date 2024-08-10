add_executable(NSLDiffTest NSLDiffTest.cpp)

target_link_libraries(NSLDiffTest labplottest)

add_test(NAME NSLDiffTest COMMAND NSLDiffTest)
