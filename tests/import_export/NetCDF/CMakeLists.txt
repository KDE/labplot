add_executable (NetCDFFilterTest NetCDFFilterTest.cpp)

target_link_libraries(NetCDFFilterTest labplot2lib labplot2test)

add_test(NAME NetCDFFilterTest COMMAND NetCDFFilterTest)
