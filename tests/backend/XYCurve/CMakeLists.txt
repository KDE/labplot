add_executable (XYCurveTest XYCurveTest.cpp)

target_link_libraries(XYCurveTest labplot2lib labplot2test)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
