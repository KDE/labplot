add_executable(NSLSmoothTest NSLSmoothTest.cpp)

target_link_libraries(NSLSmoothTest ${GSL_CBLAS_LIBRARY} labplottest ${GSL_CBLAS_LIBRARY})

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
