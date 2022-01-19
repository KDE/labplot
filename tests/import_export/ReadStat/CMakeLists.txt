add_executable (ReadStatFilterTest ReadStatFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(ReadStatFilterTest labplot2lib Qt5::Test)

add_test(NAME ReadStatFilterTest COMMAND ReadStatFilterTest)
