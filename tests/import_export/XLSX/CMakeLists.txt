add_executable(XLSXFilterTest XLSXFilterTest.cpp)

target_link_libraries(XLSXFilterTest labplottest)

add_test(NAME XLSXFilterTest COMMAND XLSXFilterTest)
