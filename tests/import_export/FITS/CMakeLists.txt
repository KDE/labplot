add_executable (FITSFilterTest FITSFilterTest.cpp)

target_link_libraries(FITSFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME FITSFilterTest COMMAND FITSFilterTest)
