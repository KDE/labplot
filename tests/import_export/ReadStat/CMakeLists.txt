add_executable(ReadStatFilterTest ReadStatFilterTest.cpp)

target_link_libraries(ReadStatFilterTest labplotmetatest)

add_test(NAME ReadStatFilterTest COMMAND ReadStatFilterTest)
