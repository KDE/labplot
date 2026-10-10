add_executable(BLFFilterTest BLFFilterTest.cpp)

target_link_libraries(BLFFilterTest labplottest)

add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
