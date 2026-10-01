add_executable(NSLSignalTest NSLSignalTest.cpp)

target_link_libraries(NSLSignalTest labplottest)

add_test(NAME NSLSignalTest COMMAND NSLSignalTest)