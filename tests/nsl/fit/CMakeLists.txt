add_executable(NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${LIBCERF_LIBRARIES} labplottest)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
