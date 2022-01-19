INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLFitTest NSLFitTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLFitTest labplot2lib Qt5::Test)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
