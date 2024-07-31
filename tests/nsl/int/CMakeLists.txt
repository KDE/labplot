add_executable(NSLIntTest NSLIntTest.cpp)

target_link_libraries(NSLIntTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
