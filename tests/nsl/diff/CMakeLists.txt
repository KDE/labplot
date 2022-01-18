INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLDiffTest NSLDiffTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLDiffTest labplot2lib Qt5::Test)

add_test(NAME NSLDiffTest COMMAND NSLDiffTest)
