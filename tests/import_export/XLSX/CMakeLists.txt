add_executable(XLSXFilterTest XLSXFilterTest.cpp)

target_link_libraries(XLSXFilterTest labplot2metatest)

add_test(NAME XLSXFilterTest COMMAND XLSXFilterTest)
