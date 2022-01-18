INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLStatsTest NSLStatsTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLStatsTest labplot2lib Qt5::Test)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)
