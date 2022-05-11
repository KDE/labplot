add_executable (ReadStatFilterTest ReadStatFilterTest.cpp)

target_link_libraries(ReadStatFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME ReadStatFilterTest COMMAND ReadStatFilterTest)
