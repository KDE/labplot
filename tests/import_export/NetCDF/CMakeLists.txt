add_executable (NetCDFFilterTest NetCDFFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(NetCDFFilterTest labplot2lib Qt5::Test)

add_test(NAME NetCDFFilterTest COMMAND NetCDFFilterTest)
