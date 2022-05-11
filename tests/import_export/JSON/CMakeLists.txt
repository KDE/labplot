add_executable (JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplot2lib Qt5::Test)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)
