INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (XYCurveTest XYCurveTest.cpp ../../CommonTest.cpp)

# enable special access variables
target_compile_definitions(XYCurveTest PRIVATE XYCurveTest_EN)
target_compile_definitions(labplot2lib PRIVATE XYCurveTest_EN)

target_link_libraries(XYCurveTest Qt5::Test labplot2lib)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
