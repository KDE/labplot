add_executable(BLFFilterTest BLFFilterTest.cpp)

target_link_libraries(BLFFilterTest labplotmetatest Vector_BLF)

add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
