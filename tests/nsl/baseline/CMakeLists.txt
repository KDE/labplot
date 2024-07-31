add_executable(NSLBaselineTest NSLBaselineTest.cpp)

target_link_libraries(NSLBaselineTest labplot2test labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

add_test(NAME NSLBaselineTest COMMAND NSLBaselineTest)
