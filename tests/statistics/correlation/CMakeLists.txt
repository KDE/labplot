add_executable (correlationtest CorrelationTest.cpp ../../CommonTest.cpp)
target_link_libraries(correlationtest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
add_test(NAME correlationtest COMMAND correlationtest)
