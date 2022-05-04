add_executable (NSLIntTest NSLIntTest.cpp)

target_link_libraries(NSLIntTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
