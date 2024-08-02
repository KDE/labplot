add_executable(NSLBaselineTest NSLBaselineTest.cpp)

target_link_libraries(NSLBaselineTest labplottest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

add_test(NAME NSLBaselineTest COMMAND NSLBaselineTest)
