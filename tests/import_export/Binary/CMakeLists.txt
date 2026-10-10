add_executable(BinaryFilterTest BinaryFilterTest.cpp)

target_link_libraries(BinaryFilterTest labplottest)

add_test(NAME BinaryFilterTest COMMAND BinaryFilterTest)
