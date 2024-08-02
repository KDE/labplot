add_executable(NSLMathTest NSLMathTest.cpp)

target_link_libraries(NSLMathTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplottest)

add_test(NAME NSLMathTest COMMAND NSLMathTest)
