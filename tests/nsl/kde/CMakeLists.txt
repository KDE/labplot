add_executable(NSLKDETest NSLKDETest.cpp)

target_link_libraries(NSLKDETest labplottest)

add_test(NAME NSLKDETest COMMAND NSLKDETest)