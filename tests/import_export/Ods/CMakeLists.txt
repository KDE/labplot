add_executable(OdsFilterTest OdsFilterTest.cpp)

target_link_libraries(OdsFilterTest labplotmetatest)

add_test(NAME OdsFilterTest COMMAND OdsFilterTest)
