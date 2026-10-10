add_executable(JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplottest)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)
