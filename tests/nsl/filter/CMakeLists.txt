INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLFilterTest NSLFilterTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLFilterTest labplot2lib Qt5::Test)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
