add_executable (CorrelationTest CorrelationTest.cpp)

target_link_libraries(CorrelationTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME CorrelationTest COMMAND CorrelationTest)
