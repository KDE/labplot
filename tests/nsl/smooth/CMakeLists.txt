add_executable(NSLSmoothTest NSLSmoothTest.cpp)

target_link_libraries(NSLSmoothTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
