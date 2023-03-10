add_executable (JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplot2lib labplot2test)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)
