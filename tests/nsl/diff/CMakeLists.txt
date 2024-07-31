add_executable(NSLDiffTest NSLDiffTest.cpp)

target_link_libraries(NSLDiffTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLDiffTest COMMAND NSLDiffTest)
