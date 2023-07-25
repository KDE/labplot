add_executable (ColumnTest ColumnTest.cpp)

target_link_libraries(ColumnTest labplot2lib labplot2test)

add_test(NAME ColumnTest COMMAND ColumnTest)
