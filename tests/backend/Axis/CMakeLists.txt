add_executable (AxisTest AxisTest.cpp)

target_link_libraries(AxisTest Qt5::Test labplot2lib labplot2test)

add_test(NAME AxisTest COMMAND AxisTest)
