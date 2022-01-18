INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (MultiRangeTest MultiRangeTest.cpp ../CommonTest.cpp)

target_link_libraries(MultiRangeTest labplot2lib Qt5::Test)

add_test(NAME MultiRangeTest COMMAND MultiRangeTest)
