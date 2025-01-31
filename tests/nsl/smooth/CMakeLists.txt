add_executable(NSLSmoothTest NSLSmoothTest.cpp)

target_link_libraries(NSLSmoothTest ${GSL_CBLAS_LIBRARY} labplottest)

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
