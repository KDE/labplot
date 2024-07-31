add_executable(NSLMathTest NSLMathTest.cpp)

target_link_libraries(NSLMathTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLMathTest COMMAND NSLMathTest)
