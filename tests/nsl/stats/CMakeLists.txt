add_executable (NSLStatsTest NSLStatsTest.cpp)

target_link_libraries(NSLStatsTest labplot2lib labplot2test)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)
