add_executable(JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplotmetatest)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)
