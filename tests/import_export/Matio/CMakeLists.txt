add_executable (MatioFilterTest MatioFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(MatioFilterTest labplot2lib Qt5::Test)

add_test(NAME MatioFilterTest COMMAND MatioFilterTest)
