INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLIntTest NSLIntTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLIntTest labplot2lib Qt5::Test)

add_test(NAME NSLIntTest COMMAND NSLIntTest)
