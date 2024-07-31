add_executable(NSLStatsTest NSLStatsTest.cpp)

target_link_libraries(NSLStatsTest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLStatsTest COMMAND NSLStatsTest)
