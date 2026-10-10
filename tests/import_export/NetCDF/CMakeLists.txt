add_executable(NetCDFFilterTest NetCDFFilterTest.cpp)

target_link_libraries(NetCDFFilterTest labplottest)

add_test(NAME NetCDFFilterTest COMMAND NetCDFFilterTest)
