add_executable(NSLBaselineTest NSLBaselineTest.cpp)

target_link_libraries(NSLBaselineTest labplottest)

add_test(NAME NSLBaselineTest COMMAND NSLBaselineTest)
