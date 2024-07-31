add_executable(MatioFilterTest MatioFilterTest.cpp)

target_link_libraries(MatioFilterTest labplot2metatest)

add_test(NAME MatioFilterTest COMMAND MatioFilterTest)
