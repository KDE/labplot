add_executable (ExcelFilterTest ExcelFilterTest.cpp)

target_link_libraries(ExcelFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME ExcelFilterTest COMMAND ExcelFilterTest)
