add_executable (NSLIntTest NSLIntTest.cpp)

target_link_libraries(NSLIntTest labplot2lib labplot2test)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
