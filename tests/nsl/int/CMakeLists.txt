add_executable(NSLIntTest NSLIntTest.cpp)

target_link_libraries(NSLIntTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplottest)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
