add_executable (MDFFilterTest MDFFilterTest.cpp)

target_link_libraries(MDFFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME MDFFilterTest COMMAND MDFFilterTest)
