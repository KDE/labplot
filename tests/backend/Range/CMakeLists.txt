INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (RangeTest RangeTest.cpp ../../CommonTest.cpp)

target_link_libraries(RangeTest Qt5::Test labplot2lib)

add_test(NAME RangeTest COMMAND RangeTest)
