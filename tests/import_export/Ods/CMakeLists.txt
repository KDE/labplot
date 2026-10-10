add_executable(OdsFilterTest OdsFilterTest.cpp)

target_link_libraries(OdsFilterTest labplottest)

add_test(NAME OdsFilterTest COMMAND OdsFilterTest)
