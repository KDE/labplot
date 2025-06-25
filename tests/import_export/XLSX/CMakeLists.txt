add_executable(XLSXFilterTest XLSXFilterTest.cpp)

target_link_libraries(XLSXFilterTest labplotmetatest)

add_test(NAME XLSXFilterTest COMMAND XLSXFilterTest)
