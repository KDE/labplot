INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (rangetest RangeTest.cpp ../../CommonTest.cpp)

target_link_libraries(rangetest Qt5::Test)

target_link_libraries(rangetest labplot2lib)

add_test(NAME rangetest COMMAND rangetest)
