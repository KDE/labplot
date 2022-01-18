INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLSmoothTest NSLSmoothTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLSmoothTest labplot2lib Qt5::Test)

add_test(NAME NSLSmoothTest COMMAND NSLSmoothTest)
