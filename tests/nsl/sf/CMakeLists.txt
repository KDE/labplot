# basic functions
add_executable(NSLSFBasicTest NSLSFBasicTest.cpp)

target_link_libraries(NSLSFBasicTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${LIBCERF_LIBRARIES} labplot2test)

add_test(NAME NSLSFBasicTest COMMAND NSLSFBasicTest)

# window functions
add_executable(NSLSFWindowTest NSLSFWindowTest.cpp)

target_link_libraries(NSLSFWindowTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLSFWindowTest COMMAND NSLSFWindowTest)
