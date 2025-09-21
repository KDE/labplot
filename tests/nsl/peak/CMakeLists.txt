add_executable(NSLPeakTest NSLPeakTest.cpp)

target_link_libraries(NSLPeakTest labplottest)

add_test(NAME NSLPeakTest COMMAND NSLPeakTest)
