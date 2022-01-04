INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (XYCurveTest XYCurveTest.cpp ../../CommonTest.cpp)

target_link_libraries(XYCurveTest Qt5::Test labplot2lib)

add_test(NAME XYCurveTest COMMAND XYCurveTest)
