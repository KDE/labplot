add_executable(NSLStatsTest NSLStatsTest.cpp)

target_link_libraries(NSLStatsTest labplottest)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)

add_executable(NSLStatisticalTestTest NSLStatisticalTestTest.cpp)

target_link_libraries(NSLStatisticalTestTest labplottest)

add_test(NAME NSLStatisticalTestTest COMMAND NSLStatisticalTestTest)

add_executable(NSLPCMTest NSLPCMTest.cpp)

target_link_libraries(NSLPCMTest labplottest)

add_test(NAME NSLPCMTest COMMAND NSLPCMTest)
