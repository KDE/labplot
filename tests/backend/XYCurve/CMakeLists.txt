add_executable (XYCurveTest XYCurveTest.cpp)

# enable special access variables
target_compile_definitions(XYCurveTest PRIVATE XYCurveTest_EN)
target_compile_definitions(labplot2lib PRIVATE XYCurveTest_EN)

target_link_libraries(XYCurveTest Qt5::Test labplot2lib labplot2test)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
