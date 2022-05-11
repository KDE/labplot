add_executable (MultiRangeTest MultiRangeTest.cpp)

target_link_libraries(MultiRangeTest labplot2lib Qt5::Test labplot2test)

add_test(NAME MultiRangeTest COMMAND MultiRangeTest)
