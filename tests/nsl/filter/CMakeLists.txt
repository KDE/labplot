add_executable(NSLFilterTest NSLFilterTest.cpp)

target_link_libraries(NSLFilterTest labplottest)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
