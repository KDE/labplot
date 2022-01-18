INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

# basic functions
add_executable (NSLSFBasicTest NSLSFBasicTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLSFBasicTest labplot2lib Qt5::Test)

add_test(NAME NSLSFBasicTest COMMAND NSLSFBasicTest)

# window functions
add_executable (NSLSFWindowTest NSLSFWindowTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLSFWindowTest labplot2lib Qt5::Test)

add_test(NAME NSLSFWindowTest COMMAND NSLSFWindowTest)
