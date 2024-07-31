add_executable(NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${LIBCERF_LIBRARIES} labplot2test)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
