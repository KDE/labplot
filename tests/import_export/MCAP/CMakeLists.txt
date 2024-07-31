add_executable(MCAPFilterTest MCAPFilterTest.cpp)

target_link_libraries(MCAPFilterTest labplot2metatest)

add_test(NAME MCAPFilterTest COMMAND MCAPFilterTest)
