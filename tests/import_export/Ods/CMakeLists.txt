add_executable(OdsFilterTest OdsFilterTest.cpp)

target_link_libraries(OdsFilterTest labplot2metatest)

add_test(NAME OdsFilterTest COMMAND OdsFilterTest)
