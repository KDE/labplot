add_executable (XLSXFilterTest XLSXFilterTest.cpp)

target_link_libraries(XLSXFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2metatest)

add_test(NAME XLSXFilterTest COMMAND XLSXFilterTest)
