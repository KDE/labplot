add_executable (MatioFilterTest MatioFilterTest.cpp)

target_link_libraries(MatioFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME MatioFilterTest COMMAND MatioFilterTest)
