add_executable (XYCurveTest XYCurveTest.cpp)

target_link_libraries(XYCurveTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
