add_executable (FITSFilterTest FITSFilterTest.cpp)

target_link_libraries(FITSFilterTest labplot2lib labplot2test)

add_test(NAME FITSFilterTest COMMAND FITSFilterTest)
