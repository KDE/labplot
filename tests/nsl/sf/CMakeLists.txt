
# basic functions
add_executable (NSLSFBasicTest NSLSFBasicTest.cpp)

target_link_libraries(NSLSFBasicTest labplot2lib labplot2test)

add_test(NAME NSLSFBasicTest COMMAND NSLSFBasicTest)

# window functions
add_executable (NSLSFWindowTest NSLSFWindowTest.cpp)

target_link_libraries(NSLSFWindowTest labplot2lib labplot2test)

add_test(NAME NSLSFWindowTest COMMAND NSLSFWindowTest)
