add_executable (StatisticalPlotsTest StatisticalPlotsTest.cpp)

target_link_libraries(StatisticalPlotsTest labplot2lib labplot2test)

add_test(NAME StatisticalPlotsTest.cpp COMMAND StatisticalPlotsTest.cpp)
