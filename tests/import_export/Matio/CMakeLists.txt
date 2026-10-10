add_executable(MatioFilterTest MatioFilterTest.cpp)

target_link_libraries(MatioFilterTest labplottest)

add_test(NAME MatioFilterTest COMMAND MatioFilterTest)
