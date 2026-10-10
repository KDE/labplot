add_executable(FITSFilterTest FITSFilterTest.cpp)

target_link_libraries(FITSFilterTest labplottest)

add_test(NAME FITSFilterTest COMMAND FITSFilterTest)
