add_executable (MultiRangeTest MultiRangeTest.cpp)

target_link_libraries(MultiRangeTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME MultiRangeTest COMMAND MultiRangeTest)
