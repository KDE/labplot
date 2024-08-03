add_executable(NSLStatsTest NSLStatsTest.cpp)

target_link_libraries(NSLStatsTest labplottest)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)
