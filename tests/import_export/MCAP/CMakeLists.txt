add_executable(MCAPFilterTest MCAPFilterTest.cpp)

target_link_libraries(MCAPFilterTest labplotmetatest)

add_test(NAME MCAPFilterTest COMMAND MCAPFilterTest)
