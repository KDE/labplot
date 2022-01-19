INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLDFTTest NSLDFTTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLDFTTest labplot2lib Qt5::Test)

add_test(NAME NSLDFTTest COMMAND NSLDFTTest)
