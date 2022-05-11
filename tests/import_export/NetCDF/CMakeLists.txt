add_executable (NetCDFFilterTest NetCDFFilterTest.cpp)

target_link_libraries(NetCDFFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME NetCDFFilterTest COMMAND NetCDFFilterTest)
