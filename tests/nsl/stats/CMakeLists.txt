add_executable (NSLStatsTest NSLStatsTest.cpp)

target_link_libraries(NSLStatsTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)
