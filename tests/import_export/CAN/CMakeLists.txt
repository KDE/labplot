add_executable(BLFFilterTest BLFFilterTest.cpp)

target_link_libraries(BLFFilterTest labplot2metatest)

add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
