add_executable(NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${LIBCERF_LIBRARIES} labplot2test)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
