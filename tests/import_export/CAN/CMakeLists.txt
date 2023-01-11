add_executable (BLFFilterTest BLFFilterTest.cpp)

target_link_libraries(BLFFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
