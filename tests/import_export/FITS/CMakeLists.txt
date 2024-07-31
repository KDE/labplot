add_executable(FITSFilterTest FITSFilterTest.cpp)

target_link_libraries(FITSFilterTest labplot2metatest)

add_test(NAME FITSFilterTest COMMAND FITSFilterTest)
