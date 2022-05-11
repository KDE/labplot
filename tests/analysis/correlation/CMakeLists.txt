add_executable (CorrelationTest CorrelationTest.cpp)

target_link_libraries(CorrelationTest labplot2lib Qt5::Test labplot2test)

add_test(NAME CorrelationTest COMMAND CorrelationTest)
