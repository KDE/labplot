add_executable(NSLMathTest NSLMathTest.cpp)

target_link_libraries(NSLMathTest labplottest)

add_test(NAME NSLMathTest COMMAND NSLMathTest)
