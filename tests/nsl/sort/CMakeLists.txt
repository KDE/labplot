add_executable(NSLSortTest NSLSortTest.cpp)

target_link_libraries(NSLSortTest labplottest)

add_test(NAME NSLSortTest COMMAND NSLSortTest)