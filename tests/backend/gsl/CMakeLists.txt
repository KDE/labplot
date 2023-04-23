add_executable (GSLTest GSLTest.cpp)

target_link_libraries(GSLTest labplot2lib labplot2test)

add_test(NAME GSLTest COMMAND GSLTest)
