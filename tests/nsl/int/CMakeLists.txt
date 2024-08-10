add_executable(NSLIntTest NSLIntTest.cpp)

target_link_libraries(NSLIntTest labplottest)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
