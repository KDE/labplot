add_executable(JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplot2metatest)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)
