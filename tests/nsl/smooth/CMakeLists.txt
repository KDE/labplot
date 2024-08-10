add_executable(NSLSmoothTest NSLSmoothTest.cpp)

target_link_libraries(NSLSmoothTest labplottest)

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
