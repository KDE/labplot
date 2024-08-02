add_executable(FITSFilterTest FITSFilterTest.cpp)

target_link_libraries(FITSFilterTest labplotmetatest)

add_test(NAME FITSFilterTest COMMAND FITSFilterTest)
