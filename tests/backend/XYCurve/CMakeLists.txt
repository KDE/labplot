add_executable (XYCurveTest XYCurveTest.cpp)

# enable special access variables
# turn it on for labplot2test instead of only XYCurveTest, because otherwise in
# other tests it is not turned on, but for the labplot2lib

target_compile_definitions(labplot2test INTERFACE XYCurveTest_EN)
target_compile_definitions(labplot2lib INTERFACE XYCurveTest_EN)

target_link_libraries(XYCurveTest Qt5::Test labplot2lib labplot2test)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
